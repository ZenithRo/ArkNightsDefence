#include "TDBaseTower.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "TDGameMode.h"
#include "TDEnemy.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "SpineSkeletonAnimationComponent.h"
#include "SpineSkeletonDataAsset.h"

ATDBaseTower::ATDBaseTower()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	TowerMesh->SetupAttachment(RootComponent);

	RangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RangeSphere"));
	RangeSphere->SetupAttachment(RootComponent);
	RangeSphere->SetSphereRadius(AttackRange);
	RangeSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MeleeRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("MeleeRangeSphere"));
	MeleeRangeSphere->SetupAttachment(RootComponent);
	MeleeRangeSphere->SetSphereRadius(100.0f);
	MeleeRangeSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SpineAnim = CreateDefaultSubobject<USpineSkeletonAnimationComponent>(TEXT("SpineAnim"));
}

void ATDBaseTower::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;
	bIsDead = false;

	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
	}

	if (SpineAnim && SkeletonDataAsset)
	{
		SpineAnim->SkeletonData = SkeletonDataAsset;
		SpineAnim->AnimationComplete.AddDynamic(this, &ATDBaseTower::OnAnimComplete);
		SpineAnim->SetAnimation(0, TEXT("Start"), false);
		AnimState = ETowerAnimState::Starting;
	}
	else if (SpineAnim)
	{
		SpineAnim->SetAutoPlay(false);
		AnimState = ETowerAnimState::Idle;
	}

	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ATDBaseTower::Fire, AttackInterval, true);

	FindTarget();
}

void ATDBaseTower::Destroyed()
{
	if (SpineAnim)
	{
		SpineAnim->AnimationComplete.RemoveDynamic(this, &ATDBaseTower::OnAnimComplete);
	}
	Super::Destroyed();
}

void ATDBaseTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead) return;

	if (!CurrentTarget || CurrentTarget->CurrentHealth <= 0.0f ||
		FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()) > AttackRange)
	{
		FindTarget();
	}

	if (CurrentTarget)
	{
		FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.0f;
		if (!Direction.IsNearlyZero())
		{
			FVector LocalDir = GetActorTransform().InverseTransformVectorNoScale(Direction);
			FVector Scale = GetActorScale3D();
			if (LocalDir.Y > 0.0f)
			{
				Scale.Y = -FMath::Abs(Scale.Y);
			}
			else if (LocalDir.Y < 0.0f)
			{
				Scale.Y = FMath::Abs(Scale.Y);
			}
			SetActorScale3D(Scale);
		}

		if (AnimState == ETowerAnimState::Idle)
		{
			PlayAnim(TEXT("Attack_Start"), false);
			AnimState = ETowerAnimState::AttackStarting;
		}
	}
	else
	{
		if (AnimState == ETowerAnimState::Attacking || AnimState == ETowerAnimState::AttackStarting)
		{
			PlayAnim(TEXT("Attack_End"), false);
			AnimState = ETowerAnimState::AttackEnding;
		}

		if (AnimState == ETowerAnimState::Idle)
		{
			FVector Scale = GetActorScale3D();
			Scale.Y = FMath::Abs(Scale.Y);
			SetActorScale3D(Scale);
		}
	}
}

void ATDBaseTower::PlayAnim(const FString& AnimName, bool Loop)
{
	if (SpineAnim && SpineAnim->HasAnimation(AnimName))
	{
		SpineAnim->SetAnimation(0, AnimName, Loop);
	}
}

void ATDBaseTower::OnAnimComplete(UTrackEntry* Entry)
{
	switch (AnimState)
	{
	case ETowerAnimState::Starting:
		PlayAnim(TEXT("Idle"), true);
		AnimState = ETowerAnimState::Idle;
		break;

	case ETowerAnimState::AttackStarting:
		PlayAnim(TEXT("Attack_Loop"), true);
		AnimState = ETowerAnimState::Attacking;
		break;

	case ETowerAnimState::AttackEnding:
		PlayAnim(TEXT("Idle"), true);
		AnimState = ETowerAnimState::Idle;
		break;

	case ETowerAnimState::Dying:
		Destroy();
		break;
	}
}

void ATDBaseTower::FindTarget()
{
	CurrentTarget = nullptr;

	UWorld* World = GetWorld();
	if (!World || bIsDead) return;

	float ClosestDist = AttackRange;

	for (TActorIterator<ATDEnemy> It(World); It; ++It)
	{
		ATDEnemy* Enemy = *It;
		if (!Enemy || Enemy->CurrentHealth <= 0.0f) continue;

		float Dist = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
		if (Dist <= ClosestDist)
		{
			ClosestDist = Dist;
			CurrentTarget = Enemy;
		}
	}
}

void ATDBaseTower::Fire()
{
	if (!CurrentTarget || CurrentTarget->CurrentHealth <= 0.0f)
	{
		FindTarget();
		if (!CurrentTarget) return;
	}

	CurrentTarget->ApplyDamage(AttackDamage, DamageType);
}

void ATDBaseTower::TakeDamage(float DamageAmount)
{
	if (bIsDead) return;

	CurrentHealth -= DamageAmount;

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ATDBaseTower::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	if (SpineAnim && SpineAnim->HasAnimation(TEXT("Die")))
	{
		PlayAnim(TEXT("Die"), false);
		AnimState = ETowerAnimState::Dying;
	}
	else
	{
		Destroy();
	}
}

bool ATDBaseTower::LevelUp()
{
	if (TowerLevel >= 3) return false;

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return false;

	int32 Cost = GetUpgradeCost();
	if (Cost <= 0) return false;

	if (!GM->SpendExperience(Cost)) return false;

	TowerLevel++;
	ApplyLevelUpStats();

	GetWorldTimerManager().ClearTimer(FireTimerHandle);
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ATDBaseTower::Fire, AttackInterval, true);

	return true;
}

int32 ATDBaseTower::GetUpgradeCost() const
{
	if (TowerLevel == 1) return UpgradeCost_Lv2;
	if (TowerLevel == 2) return UpgradeCost_Lv3;
	return 0;
}

void ATDBaseTower::ApplyLevelUpStats()
{
	AttackDamage += DamagePerLevel;
	AttackInterval = FMath::Max(0.3f, AttackInterval - IntervalReducePerLevel);
	AttackRange += RangePerLevel;

	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
	}
}
