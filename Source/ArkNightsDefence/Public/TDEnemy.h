#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDEnemy.generated.h"

class USplineComponent;
class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;

// 伤害类型: 物理伤害由DEF减免, 法术伤害由RES百分比减免
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical	UMETA(DisplayName = "物理伤害"),
	Magic		UMETA(DisplayName = "法术伤害")
};

// 敌人基类: Spline路径移动, 双防减伤, 经验掉落, 终点扣血
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

	// 脚下血条组件 (红色, 显示当前血量百分比)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> HealthBarComp;

	// 最大血量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MaxHealth = 100.0f;

	// 当前血量 (只读, 蓝图可用)
	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	float CurrentHealth;

	// 移动速度 (厘米/秒)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MoveSpeed = 300.0f;

	// 物理防御: 减免等量物理伤害 (最终伤害最低为1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Defense")
	float PhysicalArmor = 0.0f;

	// 法术抗性 (0~100): 减免百分比法术伤害 (最终伤害最低为1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Defense")
	float MagicResistance = 0.0f;

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

	// 根据伤害类型计算实际伤害并扣血 (供塔调用)
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyDamage(float DamageAmount, EDamageType DamageType);

private:
	// 缓存的Spline组件引用 (BeginPlay时从PathActor获取)
	TObjectPtr<USplineComponent> CachedSpline;

protected:
	// 死亡处理: 掉落经验 + 销毁
	UFUNCTION()
	void Die();

	// 到达终点处理: 扣玩家生命 + 销毁
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void OnReachedEnd();
};
