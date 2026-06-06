// 防御塔基类实现: 搜索目标, 旋转朝向, 定时攻击, 3档升级
#include "TDBaseTower.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "TDGameMode.h"
#include "TDEnemy.h"
#include "TDHealthBarWidget.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ATDBaseTower::ATDBaseTower()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 塔身静态网格体
	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	TowerMesh->SetupAttachment(RootComponent);

	// 攻击范围球体 (仅用于可视化)
	RangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RangeSphere"));
	RangeSphere->SetupAttachment(RootComponent);
	RangeSphere->SetSphereRadius(AttackRange);
	RangeSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 脚下蓝色血条
	HealthBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComp->SetupAttachment(RootComponent);
	HealthBarComp->SetWidgetClass(UTDHealthBarWidget::StaticClass());
	HealthBarComp->SetDrawSize(FVector2D(80.0f, 8.0f));
	HealthBarComp->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
	HealthBarComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATDBaseTower::BeginPlay()
{
	Super::BeginPlay();

	// 初始化血量
	CurrentHealth = MaxHealth;

	// 强制WidgetComponent立即创建Widget实例, 设置颜色
	if (HealthBarComp)
	{
		HealthBarComp->InitWidget();
		UTDHealthBarWidget* HB = Cast<UTDHealthBarWidget>(HealthBarComp->GetWidget());
		if (HB)
		{
			HB->SetBarColor(FLinearColor(0.1f, 0.4f, 1.0f, 1.0f));
		}
	}

	// 更新范围球体半径
	if (RangeSphere)
	{
		RangeSphere->SetSphereRadius(AttackRange);
	}

	// 启动攻击定时器: 每隔AttackInterval秒调用Fire
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ATDBaseTower::Fire, AttackInterval, true);

	// 立即搜索第一个目标, 不等下一帧Tick
	FindTarget();
}

void ATDBaseTower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 无目标或目标已超出范围 → 重新搜索
	if (!CurrentTarget || CurrentTarget->CurrentHealth <= 0.0f ||
		FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation()) > AttackRange)
	{
		FindTarget();
	}

	// 有目标时水平旋转炮塔朝向目标
	if (CurrentTarget)
	{
		FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
		Direction.Z = 0.0f;
		if (!Direction.IsNearlyZero())
		{
			FRotator TargetRot = FRotator(0.0f, Direction.Rotation().Yaw, 0.0f);
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 10.0f));
		}
	}

	// 更新脚下血条百分比 + 颜色 (蓝色)
	if (HealthBarComp)
	{
		UTDHealthBarWidget* HB = Cast<UTDHealthBarWidget>(HealthBarComp->GetUserWidgetObject());
		if (HB)
		{
			HB->SetPercent(CurrentHealth / FMath::Max(MaxHealth, 1.0f));
			HB->SetBarColor(FLinearColor(0.1f, 0.4f, 1.0f, 1.0f));
		}
	}
}

// 遍历所有敌人, 寻找攻击范围内最近的存活目标
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

// 开火: 直接对当前目标造成伤害
void ATDBaseTower::Fire()
{
	if (!CurrentTarget || CurrentTarget->CurrentHealth <= 0.0f)
	{
		FindTarget();
		if (!CurrentTarget) return;
	}

	CurrentTarget->ApplyDamage(AttackDamage, DamageType);
}

// 升级到下一级, 消耗经验, 返回是否成功
bool ATDBaseTower::LevelUp()
{
	if (TowerLevel >= 3) return false;

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return false;

	int32 Cost = GetUpgradeCost();
	if (Cost <= 0) return false;

	// 消耗经验升级
	if (!GM->SpendExperience(Cost)) return false;

	TowerLevel++;
	ApplyLevelUpStats();

	// 用新攻击间隔重启定时器
	GetWorldTimerManager().ClearTimer(FireTimerHandle);
	GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ATDBaseTower::Fire, AttackInterval, true);

	return true;
}

// 获取当前等级升级所需经验
int32 ATDBaseTower::GetUpgradeCost() const
{
	if (TowerLevel == 1) return UpgradeCost_Lv2;
	if (TowerLevel == 2) return UpgradeCost_Lv3;
	return 0;
}

// 升级属性: 加攻击, 减间隔, 加范围
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
