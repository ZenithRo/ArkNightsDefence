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
#include "Engine/DamageEvents.h"
#include "Components/WidgetComponent.h"
#include "TDHealthBarWidget.h"

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

	HealthBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComp"));
	HealthBarComp->SetupAttachment(RootComponent);
	HealthBarComp->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComp->SetDrawSize(FVector2D(120.0f, 10.0f));
	HealthBarComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	HealthBarComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarComp->SetWidgetClass(UTDHealthBarWidget::StaticClass());

	// 构造时设默认朝左, 避免被Blueprint Class Defaults覆盖
	FVector Scale = GetActorScale3D();
	Scale.Y = -FMath::Abs(Scale.Y);
	SetActorScale3D(Scale);
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
		// 先调用一次SetAnimation确保内部AnimationState已创建
		SpineAnim->SetAnimation(0, TEXT("Start"), false);
		// 再设置零混合过渡, 后续所有切换不再模糊
		if (SpineAnim->GetAnimationState())
		{
			SpineAnim->GetAnimationState()->getData()->setDefaultMix(0.0f);
		}
		// 状态机重置为Starting
		AnimState = ETowerAnimState::Starting;
	}
	else if (SpineAnim)
	{
		SpineAnim->SetAutoPlay(false);
		AnimState = ETowerAnimState::Idle;
	}

	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ATDBaseTower::Fire, AttackInterval, true);

	FindTarget();

	// 初始化血条(绿色)
	if (HealthBarComp)
	{
		if (UTDHealthBarWidget* HBWidget = Cast<UTDHealthBarWidget>(HealthBarComp->GetWidget()))
		{
			HBWidget->SetBarColor(FLinearColor(0.3f, 0.6f, 1.0f)); // 浅蓝色
		}
	}
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

	// 更新血条
	if (HealthBarComp)
	{
		UTDHealthBarWidget* HBWidget = Cast<UTDHealthBarWidget>(HealthBarComp->GetWidget());
		if (HBWidget && MaxHealth > 0.0f)
		{
			HBWidget->SetHealthPercent(CurrentHealth / MaxHealth);
		}
	}

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
			// 默认朝左(Scale.Y为负), 目标在右侧(Y>0)镜像朝右, 在左侧(Y<0)维持朝左
			FVector Scale = GetActorScale3D();
			if (Direction.Y > 0.0f)
			{
				Scale.Y = FMath::Abs(Scale.Y);
			}
			else if (Direction.Y < 0.0f)
			{
				Scale.Y = -FMath::Abs(Scale.Y);
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
			Scale.Y = -FMath::Abs(Scale.Y);
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

float ATDBaseTower::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.0f;

	CurrentHealth -= DamageAmount;

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return DamageAmount;
}

void ATDBaseTower::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	if (HealthBarComp)
	{
		HealthBarComp->SetVisibility(false);
	}

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
