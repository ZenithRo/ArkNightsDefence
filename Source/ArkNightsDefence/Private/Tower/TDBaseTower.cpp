#include "Tower/TDBaseTower.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Core/TDGameMode.h"
#include "Grid/TDGridManager.h"
#include "Enemy/TDEnemy.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "SpineSkeletonAnimationComponent.h"
#include "SpineSkeletonDataAsset.h"
#include "Engine/DamageEvents.h"
#include "Components/WidgetComponent.h"
#include "UI/TDHealthBarWidget.h"
#include "Tower/TDTargetSelector.h"

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

	// 默认攻击范围为近战1格(自身格子)
	AttackRangeCells.Add(FAttackRangeCell{0, 0});

	// 默认朝右(Scale.Y为正)
	SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
}

void ATDBaseTower::SetGridCoordinate(int32 Col, int32 Row)
{
	GridCol = Col;
	GridRow = Row;
}

void ATDBaseTower::SetDeployDirection(EDeployDirection NewDir)
{
	DeployDirection = NewDir;

	switch (NewDir)
	{
	case EDeployDirection::RIGHT:
		SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
		break;

	case EDeployDirection::LEFT:
		SetActorScale3D(FVector(1.0f, -1.0f, 1.0f));
		break;

	case EDeployDirection::UP:
		SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
		break;

	case EDeployDirection::DOWN:
		SetActorScale3D(FVector(1.0f, -1.0f, 1.0f));
		break;
	}
}

int32 ATDBaseTower::GetCurrentBlockCount() const
{
	return BlockedEnemies.Num();
}

void ATDBaseTower::AddBlockedEnemy(ATDEnemy* Enemy)
{
	if (Enemy && !BlockedEnemies.Contains(Enemy))
	{
		BlockedEnemies.Add(Enemy);
	}
}

void ATDBaseTower::RemoveBlockedEnemy(ATDEnemy* Enemy)
{
	BlockedEnemies.Remove(Enemy);
}

void ATDBaseTower::FreeAllBlockedEnemies()
{
	for (TWeakObjectPtr<ATDEnemy>& EnemyPtr : BlockedEnemies)
	{
		if (EnemyPtr.IsValid())
		{
			EnemyPtr->OnUnblocked();
		}
	}
	BlockedEnemies.Empty();
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

	// 初始化血条(浅蓝色)
	if (HealthBarComp)
	{
		HealthBarComp->InitWidget();
		if (UTDHealthBarWidget* HBWidget = Cast<UTDHealthBarWidget>(HealthBarComp->GetWidget()))
		{
			HBWidget->SetBarColor(FLinearColor(0.3f, 0.6f, 1.0f));
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
		!IsEnemyInRangeCells(CurrentTarget) ||
		FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()) > AttackRange)
	{
		FindTarget();
	}

	if (CurrentTarget)
	{
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

bool ATDBaseTower::IsEnemyInRangeCells(ATDEnemy* Enemy) const
{
	if (!Enemy || GridCol < 0 || GridRow < 0 || AttackRangeCells.Num() == 0)
	{
		return true;
	}

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager) return true;

	int32 EnemyCol, EnemyRow;
	if (!GM->GridManager->WorldToGrid(Enemy->GetActorLocation(), EnemyCol, EnemyRow))
	{
		return true;
	}

	int32 RelCol = EnemyCol - GridCol;
	int32 RelRow = EnemyRow - GridRow;

	// 根据部署方向旋转相对坐标
	int32 RotCol = RelCol, RotRow = RelRow;
	switch (DeployDirection)
	{
	case EDeployDirection::RIGHT:
		break;
	case EDeployDirection::LEFT:
		RotRow = -RelRow;
		break;
	case EDeployDirection::UP:
		RotCol = RelRow;
		RotRow = RelCol;
		break;
	case EDeployDirection::DOWN:
		RotCol = -RelRow;
		RotRow = RelCol;
		break;
	}

	for (const FAttackRangeCell& Cell : AttackRangeCells)
	{
		if (Cell.DeltaX == RotCol && Cell.DeltaY == RotRow)
		{
			return true;
		}
	}

	return false;
}

void ATDBaseTower::FindTarget()
{
	CurrentTarget = nullptr;

	UWorld* World = GetWorld();
	if (!World || bIsDead) return;

	// 收集攻击范围内所有敌人
	TArray<ATDEnemy*> Candidates;

	for (TActorIterator<ATDEnemy> It(World); It; ++It)
	{
		ATDEnemy* Enemy = *It;
		if (!Enemy || Enemy->CurrentHealth <= 0.0f) continue;
		if (!IsEnemyInRangeCells(Enemy)) continue;

		float Dist = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
		if (Dist <= AttackRange)
		{
			Candidates.Add(Enemy);
		}
	}

	if (Candidates.Num() == 0) return;

	// 使用目标选择器排序
	if (TargetSelector)
	{
		TArray<ATDEnemy*> Selected = TargetSelector->SelectTargets(Candidates, this);
		if (Selected.Num() > 0)
		{
			CurrentTarget = Selected[0];
		}
	}
	else
	{
		// 回退: 最近距离优先
		ATDEnemy* ClosestEnemy = nullptr;
		float MinDist = AttackRange;

		for (ATDEnemy* Enemy : Candidates)
		{
			float Dist = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
			if (Dist <= MinDist)
			{
				MinDist = Dist;
				ClosestEnemy = Enemy;
			}
		}

		CurrentTarget = ClosestEnemy;
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

	// 释放所有被阻挡的敌人
	FreeAllBlockedEnemies();

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
