#include "Core/TDPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Core/TDGameMode.h"
#include "Tower/TDBaseTower.h"
#include "Grid/TDGridManager.h"
#include "Grid/TDGridEnums.h"
#include "Deployment/TDDeploymentPreviewActor.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

void ATDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	if (!DefaultMappingContext) return;

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickAction)
		{
			EnhancedInput->BindAction(ClickAction, ETriggerEvent::Started, this, &ATDPlayerController::OnClick);
		}
	}
}

void ATDPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PendingDeployCountdown > 0)
	{
		PendingDeployCountdown--;
		if (PendingDeployCountdown == 0)
		{
			ExecutePendingDeploy();
			return;
		}
	}

	UpdatePreview();
}

EDeployDirection ATDPlayerController::GetDirectionFromMouse(FVector GridWorldCenter, FVector MouseWorldPos) const
{
	float dx = MouseWorldPos.X - GridWorldCenter.X;
	float dy = MouseWorldPos.Y - GridWorldCenter.Y;

	if (FMath::Abs(dx) > FMath::Abs(dy))
	{
		return dx > 0.0f ? EDeployDirection::UP : EDeployDirection::DOWN;
	}
	else
	{
		return dy > 0.0f ? EDeployDirection::RIGHT : EDeployDirection::LEFT;
	}
}

bool ATDPlayerController::GetCursorPlaneLocation(FVector& OutPlanePos) const
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP || !LP->ViewportClient || !LP->ViewportClient->Viewport) return false;

	FViewport* VP = LP->ViewportClient->Viewport;
	FVector2D CursorPos(VP->GetMouseX(), VP->GetMouseY());

	FVector WorldOrigin, WorldDir;
	if (!DeprojectScreenPositionToWorld(CursorPos.X, CursorPos.Y, WorldOrigin, WorldDir))
	{
		return false;
	}

	FHitResult Hit;
	FCollisionQueryParams QP;
	QP.bTraceComplex = false;
	FVector EndTrace = WorldOrigin + WorldDir * 50000.0f;
	if (GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, EndTrace, TDGridChannels::DeploymentPlane, QP))
	{
		OutPlanePos = Hit.Location;
		return true;
	}

	float t = -WorldOrigin.Z / WorldDir.Z;
	if (t <= 0.0f) return false;
	OutPlanePos = WorldOrigin + WorldDir * t;
	return true;
}

void ATDPlayerController::UpdatePreview()
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager)
	{
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		bHasValidHover = false;
		return;
	}

	UTDGridManager* Grid = GM->GridManager;

	if (!TowerToDeploy && !bIsDragging)
	{
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		bHasValidHover = false;
		return;
	}

	FVector PlanePos;
	if (!GetCursorPlaneLocation(PlanePos))
	{
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		bHasValidHover = false;
		return;
	}

	int32 Col, Row;
	if (!Grid->WorldToGrid(PlanePos, Col, Row))
	{
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		bHasValidHover = false;
		return;
	}

	bool bCanDeploy = TowerToDeploy
		? Grid->CanDeployAtWithPlacement(Col, Row, TowerToDeploy.GetDefaultObject()->PlacementType)
		: Grid->CanDeployAt(Col, Row);
	bCanDeploy = bCanDeploy && TowerToDeploy && GM->Cost >= TowerToDeploy.GetDefaultObject()->CostToDeploy;

	FVector CellCenter = Grid->GridToWorld(Col, Row);
	HoveredDirection = GetDirectionFromMouse(CellCenter, PlanePos);

	if (!PreviewActor && PreviewActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewActor = GetWorld()->SpawnActor<ATDDeploymentPreviewActor>(PreviewActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	if (PreviewActor)
	{
		PreviewActor->SetActorHiddenInGame(false);
		PreviewActor->SetWorldLocationAndGrid(PlanePos, Col, Row);
		PreviewActor->SetValid(bCanDeploy);
	}

	HoveredCol = Col;
	HoveredRow = Row;
	bHasValidHover = true;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Purple,
		FString::Printf(TEXT("[Preview] hover=(%d,%d) bIsDrag=%d bCanDeploy=%d"), Col, Row, bIsDragging ? 1 : 0, bCanDeploy ? 1 : 0));
}

void ATDPlayerController::ExecutePendingDeploy()
{
	PendingDeployCountdown = -1;

	if (!TowerToDeploy)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("[Deploy] cancelled: no tower"));
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		bHasValidHover = false;
		return;
	}

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager)
	{
		DeselectHandCard();
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		return;
	}

	UTDGridManager* Grid = GM->GridManager;

	FVector PlanePos;
	if (!GetCursorPlaneLocation(PlanePos))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("[Deploy] cancelled: GetCursorPlaneLocation failed"));
		DeselectHandCard();
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		return;
	}

	int32 DeployCol, DeployRow;
	if (!Grid->WorldToGrid(PlanePos, DeployCol, DeployRow))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange,
			FString::Printf(TEXT("[Deploy] cancelled: invalid cell at (%.0f,%.0f)"), PlanePos.X, PlanePos.Y));
		DeselectHandCard();
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		return;
	}

	if (!Grid->IsValidCell(DeployCol, DeployRow))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("[Deploy] cancelled: out of bounds"));
		DeselectHandCard();
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		return;
	}

	if (!Grid->CanDeployAtWithPlacement(DeployCol, DeployRow, TowerToDeploy.GetDefaultObject()->PlacementType))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("[Deploy] cancelled: invalid cell"));
		DeselectHandCard();
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		return;
	}

	if (!GM->SpendCost(TowerToDeploy.GetDefaultObject()->CostToDeploy))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("[Deploy] cancelled: cannot spend"));
		DeselectHandCard();
		return;
	}

	FVector GridCenter = Grid->GridToWorld(DeployCol, DeployRow);
	Grid->TryOccupy(DeployCol, DeployRow);

	float SurfaceZ = Grid->GetTileType(DeployCol, DeployRow) == ETileType::HIGHLAND ? 110.0f : 0.0f;
	FVector DeployLoc = FVector(GridCenter.X, GridCenter.Y, SurfaceZ);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATDBaseTower* Tower = GetWorld()->SpawnActor<ATDBaseTower>(TowerToDeploy, DeployLoc, FRotator::ZeroRotator, SpawnParams);

	if (Tower)
	{
		Tower->SetGridCoordinate(DeployCol, DeployRow);
		Tower->SetDeployDirection(HoveredDirection);
	}

	SelectedHandCardIndex = -1;
	TowerToDeploy = nullptr;
	if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
	bHasValidHover = false;
}

void ATDPlayerController::OnClick(const FInputActionValue& Value)
{
	// 部署只能通过拖拽(手牌按下→松开→5帧延迟→ExecutePendingDeploy)
	// 点击地面不部署任何东西, 避免与拖拽系统冲突
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange,
		TEXT("[OnClick] ignored - deploy via drag only"));
}

void ATDPlayerController::SelectHandCard(int32 Index)
{
	if (!HandCards.IsValidIndex(Index)) return;

	TowerToDeploy = HandCards[Index];
	SelectedHandCardIndex = Index;
}

void ATDPlayerController::BeginPlacement(int32 Index)
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
		FString::Printf(TEXT("[BeginPlacement] Index=%d"), Index));

	if (PendingDeployCountdown >= 0)
	{
		PendingDeployCountdown = -1;
	}

	if (!HandCards.IsValidIndex(Index))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
			TEXT("[BeginPlacement] FAIL: invalid index"));
		return;
	}

	TowerToDeploy = HandCards[Index];
	SelectedHandCardIndex = Index;
	bIsDragging = true;
	bHasValidHover = false;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
		FString::Printf(TEXT("[BeginPlacement] OK Index=%d Tower=%s"), Index, TowerToDeploy ? TEXT("Y") : TEXT("N")));
}

void ATDPlayerController::EndPlacement()
{
	if (!bIsDragging)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
			TEXT("[EndPlacement] SKIP: not dragging"));
		return;
	}
	bIsDragging = false;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan,
		TEXT("[EndPlacement] setting 5-frame delay"));

	if (!TowerToDeploy && SelectedHandCardIndex >= 0 && HandCards.IsValidIndex(SelectedHandCardIndex))
	{
		TowerToDeploy = HandCards[SelectedHandCardIndex];
	}

	if (!TowerToDeploy)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
			TEXT("[EndPlacement] FAIL: no tower"));
		DeselectHandCard();
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		return;
	}

	PendingDeployCountdown = 5;
}

void ATDPlayerController::DeselectHandCard()
{
	TowerToDeploy = nullptr;
	SelectedHandCardIndex = -1;
}

TSubclassOf<ATDBaseTower> ATDPlayerController::GetHandCardClass(int32 Index) const
{
	return HandCards.IsValidIndex(Index) ? HandCards[Index] : nullptr;
}

float ATDPlayerController::GetHandCardCost(int32 Index) const
{
	if (!HandCards.IsValidIndex(Index) || !HandCards[Index]) return 0.0f;
	return HandCards[Index].GetDefaultObject()->CostToDeploy;
}

TArray<int32> ATDPlayerController::GetSortedHandCardIndices() const
{
	TArray<int32> Result;
	for (int32 i = 0; i < HandCards.Num(); i++)
	{
		if (HandCards[i])
		{
			Result.Add(i);
		}
	}
	Result.Sort([this](int32 A, int32 B) {
		return GetHandCardCost(A) < GetHandCardCost(B);
	});
	return Result;
}

TArray<int32> ATDPlayerController::GetDescendingSortedHandCardIndices() const
{
	TArray<int32> Result;
	for (int32 i = 0; i < HandCards.Num(); i++)
	{
		if (HandCards[i])
		{
			Result.Add(i);
		}
	}
	Result.Sort([this](int32 A, int32 B) {
		return GetHandCardCost(A) > GetHandCardCost(B);
	});
	return Result;
}
