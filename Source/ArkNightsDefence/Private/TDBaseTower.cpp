#include "TDBaseTower.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "TDGameMode.h"
#include "TDEnemy.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

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
}

void ATDBaseTower::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
	}

	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ATDBaseTower::Fire, AttackInterval, true);

	FindTarget();
}

void ATDBaseTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
			// 将方向转换到塔的局部空间, 判断敌人在Y负半轴(左侧)还是Y正半轴(右侧)
			FVector LocalDir = GetActorTransform().InverseTransformVectorNoScale(Direction);
			FVector Scale = GetActorScale3D();
			if (LocalDir.Y > 0.0f)
			{
				Scale.X = -FMath::Abs(Scale.X);
			}
			else
			{
				Scale.X = FMath::Abs(Scale.X);
			}
			SetActorScale3D(Scale);
		}
	}
}

void ATDBaseTower::FindTarget()
{
	CurrentTarget = nullptr;

	UWorld* World = GetWorld();
	if (!World) return;

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
