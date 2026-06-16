#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TDGridManager.generated.h"

namespace TDGridChannels
{
	constexpr ECollisionChannel DeploymentPlane = ECC_GameTraceChannel1;
	constexpr ECollisionChannel MapMesh = ECC_GameTraceChannel2;
}

UENUM(BlueprintType, meta=(Bitflags))
enum class ETileType : uint8
{
	NONE     = 0,
	GROUND   = 1 << 0,
	HIGHLAND = 1 << 1,
	BLOCKED  = 1 << 2,
	START    = 1 << 3,
	END      = 1 << 4
};

USTRUCT(BlueprintType)
struct FGridCellData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bDeployable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	ETileType TileType = ETileType::GROUND;

	bool bOccupied = false;
};

UCLASS(BlueprintType, Blueprintable)
class ARKNIGHTSDEFENCE_API UTDGridManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(int32 InCols, int32 InRows, float InCellSize, FVector InOrigin);

	bool WorldToGrid(FVector WorldPos, int32& OutCol, int32& OutRow) const;

	FVector GridToWorld(int32 Col, int32 Row) const;

	bool IsValidCell(int32 Col, int32 Row) const;

	bool CanDeployAt(int32 Col, int32 Row) const;

	bool TryOccupy(int32 Col, int32 Row);

	void Free(int32 Col, int32 Row);

	bool GetDeployLocation(const APlayerController* PC, FVector& OutLocation, int32& OutCol, int32& OutRow) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FGridCellData> Cells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 NumCols = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 NumRows = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FVector GridOrigin = FVector::ZeroVector;

private:
	int32 GetIndex(int32 Col, int32 Row) const;
};
