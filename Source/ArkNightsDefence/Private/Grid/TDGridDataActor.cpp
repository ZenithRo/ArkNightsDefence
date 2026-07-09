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
