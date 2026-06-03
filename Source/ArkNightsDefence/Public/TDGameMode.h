#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TDGameMode.generated.h"

// 塔防GameMode: 管理生命/费用/经验, 提供扣费/加经验/扣生命接口
UCLASS()
class ARKNIGHTSDEFENCE_API ATDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATDGameMode();

	virtual void Tick(float DeltaTime) override;

	// 玩家生命值 (默认3点)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Lives")
	int32 PlayerLives = 3;

	// 费用上限
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Cost")
	float MaxCost = 99.0f;

	// 当前可用费用 (只读)
	UPROPERTY(BlueprintReadOnly, Category = "Game|Cost")
	float Cost;

	// 费用自然恢复速度 (每秒)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Cost")
	float CostRegenRate = 1.0f;

	// 当前作战记录/经验 (只读)
	UPROPERTY(BlueprintReadOnly, Category = "Game|Experience")
	int32 Experience;

	// HUD引用: 由BP_TDGameMode在BeginPlay创建WBP_HUD后赋值, 数据变化时自动刷新UI
	UPROPERTY(BlueprintReadWrite, Category = "Game|HUD")
	TObjectPtr<class UTDHUDWidget> HUDWidget;

	// 敌人到达终点: 扣生命, ≤0时GameOver
	UFUNCTION(BlueprintCallable, Category = "Game")
	void EnemyReachedEnd(int32 Damage);

	// 增加作战记录 (击杀敌人掉落)
	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddExperience(int32 Amount);

	// 消耗费用, 返回是否成功 (用于部署塔)
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool SpendCost(float Amount);

	// 消耗经验, 返回是否成功 (用于塔升级)
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool SpendExperience(int32 Amount);

protected:
	virtual void BeginPlay() override;

private:
	// 费用恢复定时器句柄
	FTimerHandle CostRegenTimerHandle;

	// 费用自然恢复: 每次+1.0, 不超过MaxCost
	void RegenerateCost();
};
