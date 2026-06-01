// 敌人实现: Spline路径跟随, 护甲减伤, 击杀掉落经验, 终点扣生命
#include "TDEnemy.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "TDGameMode.h"

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
}

// 受伤计算: 伤害 - 护甲, 最低1点伤害 (保证敌人可被击杀)
float ATDEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = DamageAmount - Armor;
	if (ActualDamage < 1.0f) ActualDamage = 1.0f;

	CurrentHealth -= ActualDamage;

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
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
