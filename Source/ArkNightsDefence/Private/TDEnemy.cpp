#include "TDEnemy.h"
#include "TDBaseTower.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "TDGameMode.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "SpineSkeletonAnimationComponent.h"
#include "SpineSkeletonDataAsset.h"

ATDEnemy::ATDEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetSphereRadius(50.0f);
	Collision->SetCollisionProfileName(TEXT("TDEnemy"));
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);

	SpineAnim = CreateDefaultSubobject<USpineSkeletonAnimationComponent>(TEXT("SpineAnim"));
}

void ATDEnemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	bIsDead = false;

	if (PathActor)
	{
		CachedSpline = PathActor->FindComponentByClass<USplineComponent>();
	}

	if (SpineAnim && SkeletonDataAsset)
	{
		SpineAnim->SkeletonData = SkeletonDataAsset;
		SpineAnim->AnimationComplete.AddDynamic(this, &ATDEnemy::OnAnimComplete);
		PlayAnim(TEXT("Move_Begin"), false);
		AnimState = EEnemyAnimState::MoveBeginning;
	}

	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ATDEnemy::MeleeAttack, AttackInterval, true);
}

void ATDEnemy::Destroyed()
{
	if (SpineAnim)
	{
		SpineAnim->AnimationComplete.RemoveDynamic(this, &ATDEnemy::OnAnimComplete);
	}
	Super::Destroyed();
}

void ATDEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	// 查找最近的塔
	FindNearestTower();

	// 有目标塔且在近战范围内 → 停止移动, 攻击
	if (CurrentTargetTower && !CurrentTargetTower->bIsDead &&
		FVector::Dist(GetActorLocation(), CurrentTargetTower->GetActorLocation()) <= MeleeRange)
	{
		if (AnimState == EEnemyAnimState::Moving || AnimState == EEnemyAnimState::MoveBeginning)
		{
			PlayAnim(TEXT("Move_End"), false);
			AnimState = EEnemyAnimState::MoveEnding;
		}
	}
	else if (CurrentTargetTower && !CurrentTargetTower->bIsDead)
	{
		// 有塔但超出近战范围 → 继续移动
		if (AnimState == EEnemyAnimState::Attacking || AnimState == EEnemyAnimState::MoveEnding)
		{
			PlayAnim(TEXT("Move_Begin"), false);
			AnimState = EEnemyAnimState::MoveBeginning;
		}
		CurrentTargetTower = nullptr;
	}
	else
	{
		// 无目标塔 → 如果在攻击则恢复移动
		if (AnimState == EEnemyAnimState::Attacking || AnimState == EEnemyAnimState::MoveEnding)
		{
			PlayAnim(TEXT("Move_Begin"), false);
			AnimState = EEnemyAnimState::MoveBeginning;
		}
	}

	// 当处于移动相关状态时, 沿Spline前进
	if (AnimState == EEnemyAnimState::Moving || AnimState == EEnemyAnimState::MoveBeginning)
	{
		if (!CachedSpline) return;

		DistanceAlongSpline += MoveSpeed * DeltaTime;

		float SplineLength = CachedSpline->GetSplineLength();
		FVector NewLocation = CachedSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		FRotator NewRotation = CachedSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		SetActorLocationAndRotation(NewLocation, NewRotation);

		if (DistanceAlongSpline >= SplineLength)
		{
			OnReachedEnd();
		}
	}
}

void ATDEnemy::PlayAnim(const FString& AnimName, bool Loop)
{
	if (SpineAnim && SpineAnim->HasAnimation(AnimName))
	{
		SpineAnim->SetAnimation(0, AnimName, Loop);
	}
}

void ATDEnemy::OnAnimComplete(UTrackEntry* Entry)
{
	switch (AnimState)
	{
	case EEnemyAnimState::MoveBeginning:
		PlayAnim(TEXT("Move_Loop"), true);
		AnimState = EEnemyAnimState::Moving;
		break;

	case EEnemyAnimState::MoveEnding:
		PlayAnim(TEXT("Attack"), true);
		AnimState = EEnemyAnimState::Attacking;
		break;

	case EEnemyAnimState::Dying:
		Destroy();
		break;
	}
}

void ATDEnemy::MeleeAttack()
{
	if (bIsDead) return;
	if (!CurrentTargetTower || CurrentTargetTower->bIsDead) return;
	if (AnimState != EEnemyAnimState::Attacking) return;

	CurrentTargetTower->TakeDamage(AttackDamage);
}

void ATDEnemy::FindNearestTower()
{
	if (CurrentTargetTower && !CurrentTargetTower->bIsDead) return;

	CurrentTargetTower = nullptr;

	UWorld* World = GetWorld();
	if (!World) return;

	float ClosestDist = MeleeRange;

	for (TActorIterator<ATDBaseTower> It(World); It; ++It)
	{
		ATDBaseTower* Tower = *It;
		if (!Tower || Tower->bIsDead) continue;

		float Dist = FVector::Dist(GetActorLocation(), Tower->GetActorLocation());
		if (Dist <= ClosestDist)
		{
			ClosestDist = Dist;
			CurrentTargetTower = Tower;
		}
	}
}

void ATDEnemy::ApplyDamage(float DamageAmount, EDamageType DamageType)
{
	if (bIsDead) return;

	float FinalDamage = 0.0f;

	if (DamageType == EDamageType::Physical)
	{
		FinalDamage = DamageAmount - PhysicalArmor;
	}
	else
	{
		FinalDamage = DamageAmount * (1.0f - MagicResistance / 100.0f);
	}

	if (FinalDamage < 1.0f)
	{
		FinalDamage = 1.0f;
	}

	CurrentHealth -= FinalDamage;

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ATDEnemy::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->AddExperience(ExperienceDrop);
	}

	if (SpineAnim && SpineAnim->HasAnimation(TEXT("Die")))
	{
		PlayAnim(TEXT("Die"), false);
		AnimState = EEnemyAnimState::Dying;
	}
	else
	{
		Destroy();
	}
}

void ATDEnemy::OnReachedEnd()
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->EnemyReachedEnd(LifeDamage);
	}
	Destroy();
}
