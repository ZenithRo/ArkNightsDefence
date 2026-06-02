#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDEnemy.h"
#include "TDBaseTower.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class ATDEnemy;

// 防御塔基类: 搜索最近敌人, 旋转朝向目标, 定时攻击, 3档升级
UCLASS()
class ARKNIGHTSDEFENCE_API ATDBaseTower : public AActor
{
	GENERATED_BODY()

public:
	ATDBaseTower();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 塔身静态网格体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	// 攻击范围可视化球体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> RangeSphere;

	// 攻击力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float AttackDamage = 30.0f;

	// 攻击范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float AttackRange = 300.0f;

	// 攻击间隔 (秒)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float AttackInterval = 1.0f;

	// 伤害类型 (物理/法术)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	EDamageType DamageType = EDamageType::Physical;

	// 部署消耗(费用)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float CostToDeploy = 10.0f;

	// 当前等级 (1~3)
	UPROPERTY(BlueprintReadOnly, Category = "Tower|Upgrade")
	int32 TowerLevel = 1;

	// 升到2级所需经验
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	int32 UpgradeCost_Lv2 = 50;

	// 升到3级所需经验
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	int32 UpgradeCost_Lv3 = 100;

	// 每级攻击力增量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	float DamagePerLevel = 15.0f;

	// 每级攻击间隔缩减量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	float IntervalReducePerLevel = 0.15f;

	// 每级范围增量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	float RangePerLevel = 50.0f;

	// 当前攻击目标
	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	TObjectPtr<ATDEnemy> CurrentTarget;

	// 搜索攻击范围内最近的敌人
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void FindTarget();

	// 升级 (消耗经验), 返回是否成功
	UFUNCTION(BlueprintCallable, Category = "Tower")
	bool LevelUp();

	// 开火: 对当前目标造成伤害
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void Fire();

private:
	// 攻击定时器句柄
	FTimerHandle FireTimerHandle;

	// 根据当前等级获取升级所需经验
	int32 GetUpgradeCost() const;

	// 更新属性到下一级
	void ApplyLevelUpStats();
};
