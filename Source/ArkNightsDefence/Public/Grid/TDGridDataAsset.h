#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Grid/TDGridManager.h"
#include "TDGridDataAsset.generated.h"

UCLASS(BlueprintType)
class ARKNIGHTSDEFENCE_API UTDGridDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 NumCols = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 NumRows = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FGridCellData> Cells;
};
