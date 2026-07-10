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

	if (bIsDragging)
	{
		if (!IsInputKeyDown(EKeys::LeftMouseButton))
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("[Tick] mouse released during drag, calling DoDeploy..."));
			bHasValidHover = false;
			bIsDragging = false;

			if (DoDeploy())
			{
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Deployed via drag"));
			}
			else
			{
				DeselectHandCard();
				if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
				if (GEngine)
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Placement cancelled"));
			}
			return;
		}
	}

	UpdatePreview();
}

void ATDPlayerController::BeginPlacement(int32 Index)
{
	if (!HandCards.IsValidIndex(Index)) return;

	TowerToDeploy = HandCards[Index];
	SelectedHandCardIndex = Index;
	bIsDragging = true;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
			FString::Printf(TEXT("Placement: dragging card #%d"), Index));
	}
}

void ATDPlayerController::EndPlacement()
{
	if (!bIsDragging) return;
	bHasValidHover = false;
	bIsDragging = false;

	if (DoDeploy())
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("Deployed via drag"));
	}
	else
	{
		DeselectHandCard();
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Placement cancelled"));
	}
}

bool ATDPlayerController::DoDeploy()
{
	if (!TowerToDeploy)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[DoDeploy] ✗ TowerToDeploy is null"));
		return false;
	}

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[DoDeploy] ✗ GM is null"));
		return false;
	}
	if (!GM->GridManager)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[DoDeploy] ✗ GridManager is null"));
		return false;
	}

	UTDGridManager* Grid = GM->GridManager;

	FVector DeployLoc;
	int32 Col, Row;
	if (!Grid->GetDeployLocation(this, DeployLoc, Col, Row))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[DoDeploy] ✗ GetDeployLocation failed (DeploymentPlane not hit or invalid cell)"));
		return false;
	}

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("[DoDeploy] Cell=(%d,%d) Loc=(%.0f,%.0f)"), Col, Row, DeployLoc.X, DeployLoc.Y));

	if (!Grid->CanDeployAtWithPlacement(Col, Row, TowerToDeploy.GetDefaultObject()->PlacementType))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[DoDeploy] ✗ CanDeployAtWithPlacement failed"));
		return false;
	}

	float Cost = TowerToDeploy.GetDefaultObject()->CostToDeploy;
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, FString::Printf(TEXT("[DoDeploy] Cost=%.0f GM->Cost=%.0f"), Cost, GM->Cost));

	if (!GM->SpendCost(Cost))
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[DoDeploy] ✗ SpendCost failed (not enough cost)"));
		return false;
	}

	Grid->TryOccupy(Col, Row);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATDBaseTower* Tower = GetWorld()->SpawnActor<ATDBaseTower>(TowerToDeploy, DeployLoc, FRotator::ZeroRotator, SpawnParams);

	if (!Tower)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("[DoDeploy] ✗ SpawnActor failed"));
		return false;
	}

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("[DoDeploy] ✓ Tower spawned successfully"));

	Tower->SetGridCoordinate(Col, Row);
	FVector CamLoc;
	FRotator CamRot;
	GetPlayerViewPoint(CamLoc, CamRot);
	FHitResult PlaneHit;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	FVector EndTrace = CamLoc + CamRot.Vector() * 50000.0f;
	EDeployDirection Dir = EDeployDirection::RIGHT;
	if (GetWorld()->LineTraceSingleByChannel(PlaneHit, CamLoc, EndTrace, TDGridChannels::DeploymentPlane, QueryParams))
	{
		FVector CellCenter = Grid->GridToWorld(Col, Row);
		Dir = GetDirectionFromMouse(CellCenter, PlaneHit.Location);
	}
	Tower->SetDeployDirection(Dir);

	FVector Origin, BoxExtent;
	Tower->GetActorBounds(false, Origin, BoxExtent);
	float HalfHeight = BoxExtent.Z > 1.0f ? BoxExtent.Z * 0.5f : 50.0f;
	FVector CenterLoc = DeployLoc;
	CenterLoc.Z -= HalfHeight;
	Tower->SetActorLocation(CenterLoc);

	SelectedHandCardIndex = -1;
	TowerToDeploy = nullptr;
	if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
	bHasValidHover = false;

	return true;
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

void ATDPlayerController::UpdatePreview()
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager) return;

	UTDGridManager* Grid = GM->GridManager;

	FVector DeployLoc;
	int32 Col, Row;
	bool bHit = Grid->GetDeployLocation(this, DeployLoc, Col, Row);

	if (!bHit || !TowerToDeploy)
	{
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		bHasValidHover = false;
		return;
	}

	bool bCanDeploy = Grid->CanDeployAtWithPlacement(Col, Row, TowerToDeploy.GetDefaultObject()->PlacementType);
	bCanDeploy = bCanDeploy && GM->Cost >= TowerToDeploy.GetDefaultObject()->CostToDeploy;

	FVector CamLoc;
	FRotator CamRot;
	GetPlayerViewPoint(CamLoc, CamRot);

	FHitResult PlaneHit;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	FVector EndTrace = CamLoc + CamRot.Vector() * 50000.0f;

	if (GetWorld()->LineTraceSingleByChannel(PlaneHit, CamLoc, EndTrace, TDGridChannels::DeploymentPlane, QueryParams))
	{
		FVector CellCenter = Grid->GridToWorld(Col, Row);
		HoveredDirection = GetDirectionFromMouse(CellCenter, PlaneHit.Location);
	}

	if (!PreviewActor && PreviewActorClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		PreviewActor = GetWorld()->SpawnActor<ATDDeploymentPreviewActor>(PreviewActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	if (PreviewActor)
	{
		PreviewActor->SetActorHiddenInGame(false);
		PreviewActor->SetWorldLocationAndGrid(DeployLoc, Col, Row);
		PreviewActor->SetValid(bCanDeploy);
	}

	HoveredCol = Col;
	HoveredRow = Row;
	bHasValidHover = bHit;
}

void ATDPlayerController::OnClick(const FInputActionValue& Value)
{
	if (bIsDragging)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, TEXT("[OnClick] blocked because bIsDragging"));
		return;
	}

	if (!TowerToDeploy) return;

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager) return;

	UTDGridManager* Grid = GM->GridManager;

	if (!bHasValidHover) return;
	if (!Grid->CanDeployAtWithPlacement(HoveredCol, HoveredRow, TowerToDeploy.GetDefaultObject()->PlacementType)) return;

	if (!GM->SpendCost(TowerToDeploy.GetDefaultObject()->CostToDeploy)) return;

	FVector DeployLoc;
	int32 Col, Row;
	if (!Grid->GetDeployLocation(this, DeployLoc, Col, Row)) return;
	if (Col != HoveredCol || Row != HoveredRow) return;

	Grid->TryOccupy(Col, Row);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATDBaseTower* Tower = GetWorld()->SpawnActor<ATDBaseTower>(TowerToDeploy, DeployLoc, FRotator::ZeroRotator, SpawnParams);

	if (Tower)
	{
		Tower->SetGridCoordinate(Col, Row);
		Tower->SetDeployDirection(HoveredDirection);
		FVector Origin, BoxExtent;
		Tower->GetActorBounds(false, Origin, BoxExtent);
		float HalfHeight = BoxExtent.Z > 1.0f ? BoxExtent.Z * 0.5f : 50.0f;
		FVector CenterLoc = DeployLoc;
		CenterLoc.Z -= HalfHeight;
		Tower->SetActorLocation(CenterLoc);
	}
}

void ATDPlayerController::OnRightClick()
{
	if (bIsDragging)
	{
		EndPlacement();
	}
	else if (TowerToDeploy)
	{
		DeselectHandCard();
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		bHasValidHover = false;
	}
}

void ATDPlayerController::SelectHandCard(int32 Index)
{
	if (!HandCards.IsValidIndex(Index)) return;

	TowerToDeploy = HandCards[Index];
	SelectedHandCardIndex = Index;
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
