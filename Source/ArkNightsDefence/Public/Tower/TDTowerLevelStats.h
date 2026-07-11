#pragma once

#include "CoreMinimal.h"
#include "TDTowerLevelStats.generated.h"

USTRUCT(BlueprintType)
struct FTowerLevelStats
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelStats")
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelStats")
	float PhysicalDamage = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelStats")
	float MagicDamage = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelStats")
	float AttackInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelStats")
	float CostToDeploy = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelStats")
	float PhysicalArmor = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "LevelStats")
	float MagicResistance = 0.0f;
};
