#include "Enemy/TDEnemy.h"
#include "Tower/TDBaseTower.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Core/TDGameMode.h"
#include "Grid/TDGridManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "SpineSkeletonAnimationComponent.h"
#include "SpineSkeletonDataAsset.h"
#include "Engine/DamageEvents.h"
#include "Components/WidgetComponent.h"
#include "UI/TDHealthBarWidget.h"

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

	HealthBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComp"));
	HealthBarComp->SetupAttachment(RootComponent);
	HealthBarComp->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComp->SetDrawSize(FVector2D(80.0f, 8.0f));
	HealthBarComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	HealthBarComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarComp->SetWidgetClass(UTDHealthBarWidget::StaticClass());

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

	// 初始化血条(红色)
	if (HealthBarComp)
	{
		HealthBarComp->InitWidget();
		if (UTDHealthBarWidget* HBWidget = Cast<UTDHealthBarWidget>(HealthBarComp->GetWidget()))
		{
			HBWidget->SetBarColor(FLinearColor::Red);
		}
	}
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

	// 更新血条
	if (HealthBarComp)
	{
		UTDHealthBarWidget* HBWidget = Cast<UTDHealthBarWidget>(HealthBarComp->GetWidget());
		if (HBWidget && MaxHealth > 0.0f)
		{
			HBWidget->SetHealthPercent(CurrentHealth / MaxHealth);
		}
	}

	// 地穴死亡格检测
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (GM && GM->GridManager)
	{
		int32 Col, Row;
		if (GM->GridManager->WorldToGrid(GetActorLocation(), Col, Row))
		{
			if (GM->GridManager->IsHoleCell(Col, Row))
			{
				FBox2D DeathBox = GM->GridManager->GetHoleDeathBox(Col, Row);
				FVector Loc = GetActorLocation();
				if (DeathBox.IsInside(FVector2D(Loc.X, Loc.Y)))
				{
					Die();
					return;
				}
			}
		}
	}

	// 记录移动前的位置, 用于计算移动方向
	FVector PreMoveLocation = GetActorLocation();

	// 被阻挡时检查阻挡者是否还活着
	if (bIsBlocked)
	{
		if (!BlockedByTower.IsValid() || BlockedByTower->bIsDead)
		{
			OnUnblocked();
		}
	}

	// 查找最近的塔
	FindNearestTower();

	// 判断目标是否在攻击范围内
	bool bTargetInRange = false;
	if (CurrentTargetTower && !CurrentTargetTower->bIsDead)
	{
		if (AttackRangeMode == EAttackRangeMode::Matrix && AttackRangeCells.Num() > 0)
		{
			bTargetInRange = IsTowerInRangeCells(CurrentTargetTower);
		}
		else
		{
			bTargetInRange = FVector::Dist2D(GetActorLocation(), CurrentTargetTower->GetActorLocation()) <= MeleeRange;
		}
	}

	// 有目标塔且在攻击范围内
	if (CurrentTargetTower && !CurrentTargetTower->bIsDead && bTargetInRange)
	{
		if (EnemyType != EEnemyType::Fly)
		{
			if (!bIsBlocked && CurrentTargetTower->GetCurrentBlockCount() < CurrentTargetTower->MaxBlockCount)
			{
				OnBlocked(CurrentTargetTower);
			}
		}

		if (EnemyType == EEnemyType::Fly)
		{
			if (AnimState != EEnemyAnimState::Attacking && AnimState != EEnemyAnimState::MoveEnding)
			{
				PlayAnim(TEXT("Attack"), true);
				AnimState = EEnemyAnimState::Attacking;
			}
		}
		else if (bIsBlocked)
		{
			if (AnimState == EEnemyAnimState::Moving || AnimState == EEnemyAnimState::MoveBeginning)
			{
				PlayAnim(TEXT("Move_End"), false);
				AnimState = EEnemyAnimState::MoveEnding;
			}
		}
		else
		{
			CurrentTargetTower = nullptr;
			if (AnimState == EEnemyAnimState::Attacking || AnimState == EEnemyAnimState::MoveEnding)
			{
				PlayAnim(TEXT("Move_Begin"), false);
				AnimState = EEnemyAnimState::MoveBeginning;
			}
		}
	}
	else if (CurrentTargetTower && !CurrentTargetTower->bIsDead &&
		FVector::Dist2D(GetActorLocation(), CurrentTargetTower->GetActorLocation()) <= MeleeRange)
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
		if (bIsBlocked) OnUnblocked();
	}
	else
	{
		if (AnimState == EEnemyAnimState::Attacking || AnimState == EEnemyAnimState::MoveEnding)
		{
			PlayAnim(TEXT("Move_Begin"), false);
			AnimState = EEnemyAnimState::MoveBeginning;
		}
		if (bIsBlocked) OnUnblocked();
	}

	// 当处于移动相关状态时且未被阻挡, 沿Spline前进
	// 飞行敌人在攻击时也继续移动
	if ((AnimState == EEnemyAnimState::Moving || AnimState == EEnemyAnimState::MoveBeginning ||
		(AnimState == EEnemyAnimState::Attacking && EnemyType == EEnemyType::Fly)) && !bIsBlocked)
	{
		if (!CachedSpline) return;

		float EffectiveSpeed = MoveSpeed * CurrentMoveSpeedMultiplier;
		DistanceAlongSpline += EffectiveSpeed * DeltaTime;

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
		if (spine::AnimationState* State = SpineAnim->GetAnimationState())
		{
			if (spine::TrackEntry* Entry = State->getCurrent(0))
			{
				Entry->setTimeScale(1.0f / AttackInterval);
			}
		}
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
		PlayAnim(TEXT("Attack"), false);
		AnimState = EEnemyAnimState::Attacking;
		break;

	case EEnemyAnimState::Attacking:
		MeleeAttack();
		if (CurrentTargetTower && !CurrentTargetTower->bIsDead)
		{
			PlayAnim(TEXT("Attack"), false);
			AnimState = EEnemyAnimState::Attacking;
		}
		else
		{
			CurrentTargetTower = nullptr;
		}
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

	float PhysDamage = FMath::Max(1.0f, PhysicalDamage - CurrentTargetTower->PhysicalArmor);
	float MagDamage = FMath::Max(1.0f, MagicDamage * (1.0f - CurrentTargetTower->MagicResistance / 100.0f));

	CurrentTargetTower->CurrentHealth -= (PhysDamage + MagDamage);
	if (CurrentTargetTower->CurrentHealth <= 0.0f)
	{
		CurrentTargetTower->Die();
	}
}

void ATDEnemy::ApplyDamageToSelf(float InPhysical, float InMagic)
{
	if (bIsDead) return;

	float PhysDamage = FMath::Max(1.0f, InPhysical - PhysicalArmor);
	float MagDamage = FMath::Max(1.0f, InMagic * (1.0f - MagicResistance / 100.0f));

	CurrentHealth -= (PhysDamage + MagDamage);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ATDEnemy::ApplyDamageToSelfWithPenetration(float InPhysical, float InMagic, float IgnorePhysPct, float IgnoreMagicPct)
{
	if (bIsDead) return;

	float EffectivePhysArmor = PhysicalArmor * (1.0f - FMath::Clamp(IgnorePhysPct / 100.0f, 0.0f, 1.0f));
	float EffectiveMagicResist = MagicResistance * (1.0f - FMath::Clamp(IgnoreMagicPct / 100.0f, 0.0f, 1.0f));

	float PhysDamage = FMath::Max(1.0f, InPhysical - EffectivePhysArmor);
	float MagDamage = FMath::Max(1.0f, InMagic * (1.0f - EffectiveMagicResist / 100.0f));

	CurrentHealth -= (PhysDamage + MagDamage);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}
}

void ATDEnemy::SetPathActor(AActor* InPathActor)
{
	PathActor = InPathActor;
	if (PathActor)
	{
		CachedSpline = PathActor->FindComponentByClass<USplineComponent>();
	}
}

void ATDEnemy::FindNearestTower()
{
	if (CurrentTargetTower && !CurrentTargetTower->bIsDead) return;

	CurrentTargetTower = nullptr;

	UWorld* World = GetWorld();
	if (!World) return;

	float SearchRange = (AttackRangeMode == EAttackRangeMode::Matrix && AttackRangeCells.Num() > 0)
		? 100000.0f : MeleeRange;
	float ClosestDist = SearchRange;

	for (TActorIterator<ATDBaseTower> It(World); It; ++It)
	{
		ATDBaseTower* Tower = *It;
		if (!Tower || Tower->bIsDead) continue;

		float Dist = FVector::Dist2D(GetActorLocation(), Tower->GetActorLocation());
		if (Dist > SearchRange) continue;

		if (AttackRangeMode == EAttackRangeMode::Circle)
		{
			if (Dist > MeleeRange) continue;
		}

		// 攻击目标类型过滤(地面塔/高台塔)
		if (AttackTargetType != EEnemyAttackTarget::Both)
		{
			ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
			if (GM && GM->GridManager)
			{
				ETileType TileType = GM->GridManager->GetTileType(Tower->GridCol, Tower->GridRow);
				bool bIsHighland = (TileType == ETileType::HIGHLAND);

				if (AttackTargetType == EEnemyAttackTarget::Ground && bIsHighland) continue;
				if (AttackTargetType == EEnemyAttackTarget::Highland && !bIsHighland) continue;
			}
		}

		// 矩阵模式: 检查格子是否在攻击范围内
		if (AttackRangeMode == EAttackRangeMode::Matrix && AttackRangeCells.Num() > 0)
		{
			ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
			if (GM && GM->GridManager)
			{
				int32 MyCol, MyRow, TowerCol, TowerRow;
				if (GM->GridManager->WorldToGrid(GetActorLocation(), MyCol, MyRow) &&
					GM->GridManager->WorldToGrid(Tower->GetActorLocation(), TowerCol, TowerRow))
				{
					int32 RelCol = TowerCol - MyCol;
					int32 RelRow = TowerRow - MyRow;

					bool bInRange = false;
					for (const FAttackRangeCell& Cell : AttackRangeCells)
					{
						if (Cell.DeltaX == RelCol && Cell.DeltaY == RelRow)
						{
							bInRange = true;
							break;
						}
					}
					if (!bInRange) continue;
				}
			}
		}

		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			CurrentTargetTower = Tower;
		}
	}
}

bool ATDEnemy::IsTowerInRangeCells(const ATDBaseTower* Tower) const
{
	if (!Tower || AttackRangeCells.Num() == 0) return true;

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager) return true;

	int32 MyCol, MyRow, TowerCol, TowerRow;
	if (!GM->GridManager->WorldToGrid(GetActorLocation(), MyCol, MyRow)) return false;
	if (!GM->GridManager->WorldToGrid(Tower->GetActorLocation(), TowerCol, TowerRow)) return false;

	int32 RelCol = TowerCol - MyCol;
	int32 RelRow = TowerRow - MyRow;

	for (const FAttackRangeCell& Cell : AttackRangeCells)
	{
		if (Cell.DeltaX == RelCol && Cell.DeltaY == RelRow)
		{
			return true;
		}
	}
	return false;
}

void ATDEnemy::OnBlocked(ATDBaseTower* Blocker)
{
	if (bIsBlocked || !Blocker) return;
	bIsBlocked = true;
	BlockedByTower = Blocker;
	Blocker->AddBlockedEnemy(this);
}

void ATDEnemy::OnUnblocked()
{
	if (!bIsBlocked) return;
	bIsBlocked = false;
	if (BlockedByTower.IsValid())
	{
		BlockedByTower->RemoveBlockedEnemy(this);
	}
	BlockedByTower = nullptr;
}

void ATDEnemy::ApplyDamage(float InPhysical, float InMagic)
{
	ApplyDamageToSelf(InPhysical, InMagic);
}

void ATDEnemy::ApplySlow(float SlowPercent, float Duration)
{
	if (bIsDead) return;
	CurrentMoveSpeedMultiplier = 1.0f - FMath::Clamp(SlowPercent / 100.0f, 0.0f, 1.0f);

	GetWorldTimerManager().ClearTimer(SlowTimerHandle);
	GetWorldTimerManager().SetTimer(SlowTimerHandle, this, &ATDEnemy::RemoveSlow, Duration, false);
}

void ATDEnemy::RemoveSlow()
{
	CurrentMoveSpeedMultiplier = 1.0f;
	GetWorldTimerManager().ClearTimer(SlowTimerHandle);
}

void ATDEnemy::Die()
{
	if (bIsDead) return;
	bIsDead = true;

	// 解除阻挡关系
	if (bIsBlocked && BlockedByTower.IsValid())
	{
		BlockedByTower->RemoveBlockedEnemy(this);
	}
	bIsBlocked = false;
	BlockedByTower = nullptr;

	if (HealthBarComp)
	{
		HealthBarComp->SetVisibility(false);
	}

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->AddExperience(ExperienceDrop);
	}

	OnEnemyFinished.Broadcast();

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

	OnEnemyReachedEndDel.Broadcast();
	Destroy();
}
