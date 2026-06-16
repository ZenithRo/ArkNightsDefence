#include "TDGridDataActor.h"
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

	SetActorTickEnabled(false);
	SetActorHiddenInGame(true);
}

void ATDGridDataActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawEditorGrid();
}

void ATDGridDataActor::DrawEditorGrid()
{

	if (NumCols <= 0 || NumRows <= 0 || CellSize <= 0.0f) return;

	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 Row = 0; Row < NumRows; Row++)
	{
		for (int32 Col = 0; Col < NumCols; Col++)
		{
			int32 Idx = Row * NumCols + Col;
			bool bDeployable = Cells.IsValidIndex(Idx) ? Cells[Idx].bDeployable : false;

			float CenterX = GridOrigin.X + (Col + 0.5f) * CellSize;
			float CenterY = GridOrigin.Y + (Row + 0.5f) * CellSize;
			FVector Center(CenterX, CenterY, DrawHeight);
			FVector Extent(CellSize * 0.45f, CellSize * 0.45f, 10.0f);

			FColor Color = bDeployable ? FColor::Green : FColor::Red;

			DrawDebugBox(World, Center, Extent, Color, false, -1.0f, 0, 2.0f);
			DrawDebugBox(World, Center, Extent, Color, false, -1.0f, 0, 2.0f);
		}
	}
}

void ATDGridDataActor::ApplyToGridManager(UTDGridManager* Manager) const
{
	if (!Manager) return;

	Manager->Initialize(NumCols, NumRows, CellSize, GridOrigin);

	for (int32 i = 0; i < Cells.Num() && i < Manager->Cells.Num(); i++)
	{
		Manager->Cells[i].bDeployable = Cells[i].bDeployable;
	}
}
