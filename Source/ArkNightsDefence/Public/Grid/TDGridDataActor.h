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

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ExportToDataAsset(UTDGridDataAsset* Asset) const;

	// 通过行列坐标设置格子类型(编辑器工具用)
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetCellType(int32 Col, int32 Row, ETileType NewType);

	// 获取指定格子的类型
	UFUNCTION(BlueprintCallable, Category = "Grid")
	ETileType GetCellType(int32 Col, int32 Row) const;

	// 调整网格大小并重置所有格子为默认
	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetGridSize(int32 NewCols, int32 NewRows);

	// 世界坐标→格子坐标(编辑器工具用)
	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool WorldToGrid(FVector WorldPos, int32& OutCol, int32& OutRow) const;

	// 格子坐标→世界中心坐标
	UFUNCTION(BlueprintCallable, Category = "Grid")
	FVector GridToWorld(int32 Col, int32 Row) const;

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

	// ——— 编辑器工具控制 ———
	// 启用/禁用编辑器画笔工具
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|EditorTool")
	bool bEditorToolEnabled = false;

	// 当前笔刷类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|EditorTool")
	ETileType EditorBrushType = ETileType::GROUND;

protected:
	virtual void BeginPlay() override;

	virtual void PostInitProperties() override;

	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	void DrawEditorGrid();

private:
	bool IsValidCellCoords(int32 Col, int32 Row) const;
};
