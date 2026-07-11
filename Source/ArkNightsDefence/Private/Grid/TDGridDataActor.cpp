#include "Grid/TDGridDataActor.h"
#include "Grid/TDGridDataAsset.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// 仅在编辑器构建中使用GEditor相关API
#if WITH_EDITOR
#include "EditorViewportClient.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/Engine.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogGridEditor, Log, All);

ATDGridDataActor::ATDGridDataActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATDGridDataActor::BeginPlay()
{
	Super::BeginPlay();

	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
}

void ATDGridDataActor::PostInitProperties()
{
	Super::PostInitProperties();

	if (Cells.Num() == 0)
	{
		SetGridSize(NumCols, NumRows);
	}
	else if (GridDataAsset)
	{
		ImportFromDataAsset(GridDataAsset);
	}
}

void ATDGridDataActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawEditorGrid();

#if WITH_EDITOR
	if (bEditorToolEnabled && GEditor)
	{
		FViewport* ActiveViewport = GEditor->GetActiveViewport();
		if (!ActiveViewport) return;

		FEditorViewportClient* ViewportClient = (FEditorViewportClient*)ActiveViewport->GetClient();
		if (!ViewportClient) return;

		FIntPoint MousePos;
		ActiveViewport->GetMousePos(MousePos);

		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
			ActiveViewport, ViewportClient->GetScene(), ViewportClient->EngineShowFlags));
		FSceneView* View = ViewportClient->CalcSceneView(&ViewFamily);

		if (View)
		{
			FVector Origin, Direction;
			View->DeprojectFVector2D(FVector2D(MousePos.X, MousePos.Y), Origin, Direction);

			if (FMath::Abs(Direction.Z) > 1e-6f)
			{
				float t = -Origin.Z / Direction.Z;
				FVector HitLocation = Origin + Direction * t;

				int32 Col, Row;
				if (WorldToGrid(HitLocation, Col, Row))
				{
					if (ActiveViewport->KeyState(EKeys::LeftMouseButton))
					{
						SetCellType(Col, Row, EditorBrushType);
						if (!IsRunningGame())
						{
							Modify();
							PostEditChange();
						}
					}
				}
			}
		}
	}
#endif
}

static FColor GetTileColor(ETileType TileType)
{
	switch (TileType)
	{
	case ETileType::GROUND:		return FColor(0, 180, 0);
	case ETileType::HIGHLAND:	return FColor(0, 60, 180);
	case ETileType::BLOCKED:	return FColor(120, 120, 120);
	case ETileType::HOLE:		return FColor(180, 0, 0);
	default:					return FColor(80, 80, 80);
	}
}

void ATDGridDataActor::DrawEditorGrid()
{
	if (NumCols <= 0 || NumRows <= 0 || CellSize <= 0.0f) return;

	UWorld* World = GetWorld();
	if (!World) return;

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

	Manager->Initialize(NumCols, NumRows, CellSize, GridOrigin);

	for (int32 i = 0; i < Manager->Cells.Num(); i++)
	{
		if (Cells.IsValidIndex(i))
		{
			Manager->Cells[i].bDeployable = Cells[i].bDeployable;
			Manager->Cells[i].TileType = Cells[i].TileType;
		}
		else
		{
			Manager->Cells[i].bDeployable = false;
			Manager->Cells[i].TileType = ETileType::BLOCKED;
		}
	}
}

void ATDGridDataActor::ImportFromDataAsset(const UTDGridDataAsset* Asset)
{
	if (!Asset) return;

	NumCols = Asset->NumCols;
	NumRows = Asset->NumRows;
	CellSize = Asset->CellSize;

	Cells.Empty();
	Cells.SetNum(NumCols * NumRows);
	for (int32 i = 0; i < Cells.Num(); i++)
	{
		if (Asset->Cells.IsValidIndex(i))
		{
			Cells[i] = Asset->Cells[i];
		}
		else
		{
			Cells[i].TileType = ETileType::BLOCKED;
			Cells[i].bDeployable = false;
		}
	}
}

void ATDGridDataActor::ExportToDataAsset(UTDGridDataAsset* Asset) const
{
	if (!Asset) return;

	Asset->NumCols = NumCols;
	Asset->NumRows = NumRows;
	Asset->CellSize = CellSize;
	Asset->Cells = Cells;
}

void ATDGridDataActor::SetCellType(int32 Col, int32 Row, ETileType NewType)
{
	if (!IsValidCellCoords(Col, Row)) return;

	int32 Idx = Row * NumCols + Col;
	if (Cells.IsValidIndex(Idx))
	{
		Cells[Idx].TileType = NewType;
		Cells[Idx].bDeployable = (NewType != ETileType::BLOCKED && NewType != ETileType::HOLE);
	}
}

ETileType ATDGridDataActor::GetCellType(int32 Col, int32 Row) const
{
	if (!IsValidCellCoords(Col, Row)) return ETileType::GROUND;

	int32 Idx = Row * NumCols + Col;
	return Cells.IsValidIndex(Idx) ? Cells[Idx].TileType : ETileType::GROUND;
}

void ATDGridDataActor::SetGridSize(int32 NewCols, int32 NewRows)
{
	if (NewCols <= 0 || NewRows <= 0) return;

	NumCols = NewCols;
	NumRows = NewRows;

	Cells.Empty();
	Cells.SetNum(NumCols * NumRows);
	for (int32 i = 0; i < Cells.Num(); i++)
	{
		Cells[i].TileType = ETileType::BLOCKED;
		Cells[i].bDeployable = false;
	}
}

bool ATDGridDataActor::IsValidCellCoords(int32 Col, int32 Row) const
{
	return Col >= 0 && Row >= 0 && Col < NumCols && Row < NumRows;
}

bool ATDGridDataActor::WorldToGrid(FVector WorldPos, int32& OutCol, int32& OutRow) const
{
	if (CellSize <= 0.0f) return false;

	FVector Corner = GridOrigin - FVector(NumCols * CellSize * 0.5f, NumRows * CellSize * 0.5f, 0.0f);

	float RelX = WorldPos.X - Corner.X;
	float RelY = WorldPos.Y - Corner.Y;

	OutCol = FMath::FloorToInt(RelX / CellSize);
	OutRow = FMath::FloorToInt(RelY / CellSize);

	return IsValidCellCoords(OutCol, OutRow);
}

FVector ATDGridDataActor::GridToWorld(int32 Col, int32 Row) const
{
	FVector Corner = GridOrigin - FVector(NumCols * CellSize * 0.5f, NumRows * CellSize * 0.5f, 0.0f);

	float CenterX = Corner.X + (Col + 0.5f) * CellSize;
	float CenterY = Corner.Y + (Row + 0.5f) * CellSize;
	return FVector(CenterX, CenterY, GridOrigin.Z);
}
