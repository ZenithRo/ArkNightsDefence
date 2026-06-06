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
#include "Engine/DamageEvents.h"

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

	// 构造时设默认朝左, 避免被Blueprint Class Defaults覆盖
	FVector Scale = GetActorScale3D();
	Scale.Y = -FMath::Abs(Scale.Y);
	SetActorScale3D(Scale);
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
		// 先调用一次SetAnimation确保内部AnimationState已创建
		PlayAnim(TEXT("Move_Begin"), false);
		// 再设置零混合过渡, 后续所有切换不再模糊
		if (SpineAnim->GetAnimationState())
		{
			SpineAnim->GetAnimationState()->getData()->setDefaultMix(0.0f);
		}
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

	// 记录移动前的位置, 用于计算移动方向
	FVector PreMoveLocation = GetActorLocation();

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
		if (AnimState == EEnemyAnimState::Attacking || AnimState == EEnemyAnimState::MoveEnding)
		{
			PlayAnim(TEXT("Move_Begin"), false);
			AnimState = EEnemyAnimState::MoveBeginning;
		}
		CurrentTargetTower = nullptr;
	}
	else
	{
		if (AnimState == EEnemyAnimState::Attacking || AnimState == EEnemyAnimState::MoveEnding)
		{
			PlayAnim(TEXT("Move_Begin"), false);
			AnimState = EEnemyAnimState::MoveBeginning;
		}
	}

	// 当处于移动相关状态时, 沿Spline前进 (只取位置, 不旋转Actor)
	if (AnimState == EEnemyAnimState::Moving || AnimState == EEnemyAnimState::MoveBeginning)
	{
		if (!CachedSpline) return;

		DistanceAlongSpline += MoveSpeed * DeltaTime;

		float SplineLength = CachedSpline->GetSplineLength();
		FVector NewLocation = CachedSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
		SetActorLocation(NewLocation);

		if (DistanceAlongSpline >= SplineLength)
		{
			OnReachedEnd();
		}
	}

	// 仅根据世界空间Y方向决定左右翻转 (默认Scale.Y为负朝左)
	// 向左移动(MoveDelta.Y<0)保持朝左, 向右移动(MoveDelta.Y>0)镜像朝右
	FVector MoveDelta = GetActorLocation() - PreMoveLocation;
	MoveDelta.Z = 0.0f;
	if (!MoveDelta.IsNearlyZero())
	{
		FVector Scale = GetActorScale3D();
		if (MoveDelta.Y > 0.0f)
		{
			Scale.Y = FMath::Abs(Scale.Y);
		}
		else if (MoveDelta.Y < 0.0f)
		{
			Scale.Y = -FMath::Abs(Scale.Y);
		}
		SetActorScale3D(Scale);
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

	CurrentTargetTower->TakeDamage(AttackDamage, FDamageEvent(), nullptr, nullptr);
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
