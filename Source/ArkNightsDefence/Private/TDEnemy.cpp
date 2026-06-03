// 敌人实现: Spline路径跟随, 物理/法术双防减伤, 击杀掉落经验, 终点扣生命
#include "TDEnemy.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Components/WidgetComponent.h"
#include "TDGameMode.h"
#include "TDHealthBarWidget.h"

ATDEnemy::ATDEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	// 球体碰撞体作为根组件, 用于后续与塔弹丸碰撞检测
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetSphereRadius(50.0f);
	Collision->SetCollisionProfileName(TEXT("TDEnemy"));
	RootComponent = Collision;

	// 网格体挂载到碰撞体下
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);

	// 脚下红色血条
	HealthBarComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComp->SetupAttachment(Collision);
	HealthBarComp->SetWidgetClass(UTDHealthBarWidget::StaticClass());
	HealthBarComp->SetDrawSize(FVector2D(80.0f, 8.0f));
	HealthBarComp->SetRelativeLocation(FVector(0.0f, 0.0f, -55.0f));
	HealthBarComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATDEnemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	// 从PathActor中查找Spline组件并缓存, 避免每帧FindComponent
	if (PathActor)
	{
		CachedSpline = PathActor->FindComponentByClass<USplineComponent>();
	}
}

// 每帧沿Spline移动: 推进距离 → 更新位置+旋转 → 检测是否到达终点
void ATDEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CachedSpline) return;

	// 沿路径推进, 速度 × 帧时间
	DistanceAlongSpline += MoveSpeed * DeltaTime;

	// 从Spline获取世界坐标位置和朝向
	float SplineLength = CachedSpline->GetSplineLength();
	FVector NewLocation = CachedSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	FRotator NewRotation = CachedSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	SetActorLocationAndRotation(NewLocation, NewRotation);

	// 到达终点 → 扣玩家生命
	if (DistanceAlongSpline >= SplineLength)
	{
		OnReachedEnd();
	}

	// 更新脚下血条百分比 + 颜色 (红色)
	if (HealthBarComp)
	{
		UTDHealthBarWidget* HB = Cast<UTDHealthBarWidget>(HealthBarComp->GetUserWidgetObject());
		if (HB)
		{
			HB->SetPercent(CurrentHealth / FMath::Max(MaxHealth, 1.0f));
			HB->SetBarColor(FLinearColor(1.0f, 0.1f, 0.1f, 1.0f));
		}
	}
}

// 根据伤害类型计算实际伤害并扣血
// 物理: 伤害 - 物理防御, 最低1点
// 法术: 伤害 × (1 - 法术抗性/100), 最低1点
void ATDEnemy::ApplyDamage(float DamageAmount, EDamageType DamageType)
{
	float FinalDamage = 0.0f;

	if (DamageType == EDamageType::Physical)
	{
		// 物理伤害 = 攻击力 - 防御力 (最低1点)
		FinalDamage = DamageAmount - PhysicalArmor;
	}
	else // Magic
	{
		// 法术伤害 = 攻击力 × (1 - 抗性%) (最低1点)
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

// 死亡: 掉落经验 → 销毁自身
void ATDEnemy::Die()
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->AddExperience(ExperienceDrop);
	}
	Destroy();
}

// 到达终点: 扣减玩家生命 → 销毁自身
void ATDEnemy::OnReachedEnd()
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->EnemyReachedEnd(LifeDamage);
	}
	Destroy();
}
