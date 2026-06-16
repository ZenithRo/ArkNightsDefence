#include "Enemy/TDEnemy.h"
#include "Tower/TDBaseTower.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Core/TDGameMode.h"
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

	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ATDEnemy::MeleeAttack, AttackInterval, true);

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

	// 有目标塔且在近战范围内
	if (CurrentTargetTower && !CurrentTargetTower->bIsDead &&
		FVector::Dist(GetActorLocation(), CurrentTargetTower->GetActorLocation()) <= MeleeRange)
	{
		// 未被阻挡且塔有阻挡空位 → 触发阻挡
		if (!bIsBlocked && CurrentTargetTower->GetCurrentBlockCount() < CurrentTargetTower->MaxBlockCount)
		{
			OnBlocked(CurrentTargetTower);
		}

		if (bIsBlocked)
		{
			// 被阻挡 → 停止移动, 攻击
			if (AnimState == EEnemyAnimState::Moving || AnimState == EEnemyAnimState::MoveBeginning)
			{
				PlayAnim(TEXT("Move_End"), false);
				AnimState = EEnemyAnimState::MoveEnding;
			}
		}
		else
		{
			// 塔阻挡已满, 穿过
			CurrentTargetTower = nullptr;
			if (AnimState == EEnemyAnimState::Attacking || AnimState == EEnemyAnimState::MoveEnding)
			{
				PlayAnim(TEXT("Move_Begin"), false);
				AnimState = EEnemyAnimState::MoveBeginning;
			}
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

	// 当处于移动相关状态时且未被阻挡, 沿Spline前进 (只取位置, 不旋转Actor)
	if ((AnimState == EEnemyAnimState::Moving || AnimState == EEnemyAnimState::MoveBeginning) && !bIsBlocked)
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

void ATDEnemy::OnBlocked(ATDBaseTower* Blocker)
{
	if (bIsBlocked) return;
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

	// 解除阻挡关系
	if (bIsBlocked && BlockedByTower.IsValid())
	{
		BlockedByTower->RemoveBlockedEnemy(this);
	}
	bIsBlocked = false;
	BlockedByTower = nullptr;

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);

	if (HealthBarComp)
	{
		HealthBarComp->SetVisibility(false);
	}

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
