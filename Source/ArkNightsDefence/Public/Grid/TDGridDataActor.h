#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/TDGridManager.h"
#include "TDGridDataActor.generated.h"

class UTDGridDataAsset;

UCLASS(Blueprintable)
class ARKNIGHTSDEFENCE_API ATDGridDataActor : public AActor
{
	GENERATED_BODY()

public:
	ATDGridDataActor();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ApplyToGridManager(UTDGridManager* Manager) const;

	// 从DataAsset导入格子数据
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ImportFromDataAsset(const UTDGridDataAsset* Asset);

	// 导出到DataAsset
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ExportToDataAsset(UTDGridDataAsset* Asset) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 NumCols = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 NumRows = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FVector GridOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FGridCellData> Cells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float DrawHeight = 0.0f;

	// 关联的网格数据资产(支持多关卡独立管理)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TObjectPtr<UTDGridDataAsset> GridDataAsset;

protected:
	virtual void BeginPlay() override;

	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	void DrawEditorGrid();
};
