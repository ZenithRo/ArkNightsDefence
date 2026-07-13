#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TDWaveTypes.generated.h"

class ATDEnemy;

USTRUCT(BlueprintType)
struct FTDWaveSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSubclassOf<ATDEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 Count = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 PathIndex = 0;
};

USTRUCT(BlueprintType)
struct FTDWaveTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float WaveStartDelay = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FTDWaveSpawnEntry> SpawnEntries;
};
