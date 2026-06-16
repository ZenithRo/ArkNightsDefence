#include "Grid/TDGridDataActor.h"
#include "Grid/TDGridDataAsset.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATDGridDataActor::ATDGridDataActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATDGridDataActor::BeginPlay()
{
	Super::BeginPlay();

	// 如果关联了DataAsset且Cells为空, 自动从DataAsset加载
	if (GridDataAsset && Cells.Num() == 0)
	{
		ImportFromDataAsset(GridDataAsset);
	}

	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
}

void ATDGridDataActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawEditorGrid();
}

static FColor GetTileColor(ETileType TileType)
{
	switch (TileType)
	{
	case ETileType::GROUND:		return FColor(0, 180, 0);	// 浅绿
	case ETileType::HIGHLAND:	return FColor(0, 60, 180);	// 深蓝
	case ETileType::BLOCKED:	return FColor(120, 120, 120);// 灰
	case ETileType::HOLE:		return FColor(180, 0, 0);	// 深红
	default:					return FColor(80, 80, 80);
	}
}

void ATDGridDataActor::DrawEditorGrid()
{
	if (NumCols <= 0 || NumRows <= 0 || CellSize <= 0.0f) return;

	UWorld* World = GetWorld();
	if (!World) return;

	// 将网格中心转为左上角坐标
	FVector Corner = GridOrigin - FVector(NumCols * CellSize * 0.5f, NumRows * CellSize * 0.5f, 0.0f);

	for (int32 Row = 0; Row < NumRows; Row++)
	{
		for (int32 Col = 0; Col < NumCols; Col++)
		{
			int32 Idx = Row * NumCols + Col;
			ETileType TileType = Cells.IsValidIndex(Idx) ? Cells[Idx].TileType : ETileType::GROUND;
			FColor Color = GetTileColor(TileType);

			float CenterX = Corner.X + (Col + 0.5f) * CellSize;
			float CenterY = Corner.Y + (Row + 0.5f) * CellSize;
			FVector Center(CenterX, CenterY, DrawHeight);
			FVector Extent(CellSize * 0.45f, CellSize * 0.45f, 10.0f);

			DrawDebugBox(World, Center, Extent, Color, false, -1.0f, 0, 2.0f);

			// 地穴格额外绘制90%死亡判定矩形线框
			if (TileType == ETileType::HOLE)
			{
				float DeathHalfSize = CellSize * 0.9f * 0.5f;
				FVector DeathExtent(DeathHalfSize, DeathHalfSize, 10.0f);
				DrawDebugBox(World, Center, DeathExtent, FColor(255, 100, 0), false, -1.0f, 0, 4.0f);
			}
		}
	}
}

void ATDGridDataActor::ApplyToGridManager(UTDGridManager* Manager) const
{
	if (!Manager) return;

	// 将网格中心转为左上角坐标传给GridManager
	FVector Corner = GridOrigin - FVector(NumCols * CellSize * 0.5f, NumRows * CellSize * 0.5f, 0.0f);
	Manager->Initialize(NumCols, NumRows, CellSize, Corner);

	for (int32 i = 0; i < Cells.Num() && i < Manager->Cells.Num(); i++)
	{
		Manager->Cells[i].bDeployable = Cells[i].bDeployable;
		Manager->Cells[i].TileType = Cells[i].TileType;
	}
}

void ATDGridDataActor::ImportFromDataAsset(const UTDGridDataAsset* Asset)
{
	if (!Asset) return;

	NumCols = Asset->NumCols;
	NumRows = Asset->NumRows;
	CellSize = Asset->CellSize;
	Cells = Asset->Cells;
}

void ATDGridDataActor::ExportToDataAsset(UTDGridDataAsset* Asset) const
{
	if (!Asset) return;

	Asset->NumCols = NumCols;
	Asset->NumRows = NumRows;
	Asset->CellSize = CellSize;
	Asset->Cells = Cells;
}
