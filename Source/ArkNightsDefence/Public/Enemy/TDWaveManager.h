#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDWaveManager.generated.h"

class ATDEnemy;
class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveProgressDelegate, int32, Killed, int32, Total);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveChangedDelegate, int32, WaveIndex);

UCLASS(Blueprintable)
class ARKNIGHTSDEFENCE_API ATDWaveManager : public AActor
{
    GENERATED_BODY()

public:
    ATDWaveManager();

	// 波次配置表 (DataTable行名: Wave_1, Wave_2, ...)
	UPROPERTY(EditAnywhere, Category = "Waves")
	TObjectPtr<UDataTable> WaveDataTable;

    // 路径Actor列表 (PathIndex映射到该数组)
    UPROPERTY(EditAnywhere, Category = "Paths")
    TArray<TObjectPtr<AActor>> PathActors;

    FOnWaveProgressDelegate OnWaveProgress;
    FOnWaveChangedDelegate OnWaveChanged;

    UFUNCTION(BlueprintCallable, Category = "Waves")
    void StartAllWaves();

    UFUNCTION(BlueprintCallable, Category = "Waves")
    void OnEnemyKilled();

    UFUNCTION(BlueprintCallable, Category = "Waves")
    void OnEnemyReachedEnd();

    UFUNCTION(BlueprintPure, Category = "Waves")
    int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

    UFUNCTION(BlueprintPure, Category = "Waves")
    int32 GetKilledCount() const { return KilledCount; }

    UFUNCTION(BlueprintPure, Category = "Waves")
    int32 GetTotalCount() const { return TotalCount; }

    // 所有波次中的敌人都已被击杀或到达终点时为 true。
    UFUNCTION(BlueprintPure, Category = "Waves")
    bool AreAllWavesCompleted() const { return bAllWavesCompleted; }

    // 由 GameMode 在胜负结算时调用，停止后续生成计时器。
    UFUNCTION(BlueprintCallable, Category = "Waves")
    void StopAllWaves();

protected:
    virtual void BeginPlay() override;

private:
    int32 CurrentWaveIndex = 0;
    int32 KilledCount = 0;
    int32 TotalCount = 0;
    int32 WaveTotalEnemies = 0;
    int32 WaveProcessedEnemies = 0;
	bool bAllWavesCompleted = false;

    struct FWaveSpawnState
    {
        TSubclassOf<ATDEnemy> EnemyClass;
        int32 Remaining = 0;
        float SpawnInterval = 1.0f;
        int32 PathIndex = 0;
    };

    TArray<FWaveSpawnState> ActiveSpawnStates;
    FTimerHandle SpawnTimerHandle;

    TArray<FName> WaveRowNames;
    void StartWave(int32 WaveIndex);
    void SpawnNextFromState();
    void CheckWaveComplete();
};
