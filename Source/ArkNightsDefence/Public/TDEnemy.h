#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDEnemy.generated.h"

class USplineComponent;
class USphereComponent;
class UStaticMeshComponent;

// 敌人基类: Spline路径移动, 护甲减伤, 经验掉落, 终点扣血
UCLASS()
class ARKNIGHTSDEFENCE_API ATDEnemy : public AActor
{
	GENERATED_BODY()

public:
	ATDEnemy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 球体碰撞组件 (RootComponent)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	// 静态网格体组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// 最大血量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MaxHealth = 100.0f;

	// 当前血量 (只读, 蓝图可用)
	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	float CurrentHealth;

	// 移动速度 (厘米/秒)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MoveSpeed = 300.0f;

	// 护甲值: 减免等量伤害 (最低1点伤害)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float Armor = 0.0f;

	// 击杀后掉落的作战记录数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 ExperienceDrop = 10;

	// 到达终点时对玩家生命造成的伤害
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 LifeDamage = 1;

	// 路径Actor引用 (蓝图绑定BP_Path)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	TObjectPtr<AActor> PathActor;

	// 当前沿路径距离 (只读, 蓝图可见)
	UPROPERTY(BlueprintReadOnly, Category = "Path")
	float DistanceAlongSpline = 0.0f;

private:
	// 缓存的Spline组件引用 (BeginPlay时从PathActor获取)
	TObjectPtr<USplineComponent> CachedSpline;

	// 重写TakeDamage: 实现护甲减伤逻辑
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	// 死亡处理: 掉落经验 + 销毁
	UFUNCTION()
	void Die();

	// 到达终点处理: 扣玩家生命 + 销毁
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void OnReachedEnd();
};
