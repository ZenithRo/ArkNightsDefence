#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDWaveManager.generated.h"

class ATDEnemy;

USTRUCT(BlueprintType)
struct FWaveEnemyEntry
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Wave")
    TSubclassOf<ATDEnemy> EnemyClass;

    UPROPERTY(EditDefaultsOnly, Category = "Wave")
    float SpawnDelay = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Wave")
    int32 PathIndex = 0;
};

USTRUCT(BlueprintType)
struct FWaveData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Wave")
    TArray<FWaveEnemyEntry> Enemies;

    UPROPERTY(EditDefaultsOnly, Category = "Wave")
    float WaveStartDelay = 0.0f;
};

UCLASS(Blueprintable)
class ARKNIGHTSDEFENCE_API ATDWaveManager : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Waves")
    TArray<FWaveData> WaveConfigs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Waves")
    int32 CurrentWaveIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Waves")
    void StartAllWaves();

    UFUNCTION(BlueprintCallable, Category = "Waves")
    void SpawnWave(int32 WaveIndex);

protected:
    virtual void BeginPlay() override;
};
