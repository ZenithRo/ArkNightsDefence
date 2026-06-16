#include "Core/TDPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Core/TDGameMode.h"
#include "Tower/TDBaseTower.h"
#include "Grid/TDGridManager.h"
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

void ATDPlayerController::UpdatePreview()
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager) return;

	UTDGridManager* Grid = GM->GridManager;

	FVector DeployLoc;
	int32 Col, Row;
	bool bHit = Grid->GetDeployLocation(this, DeployLoc, Col, Row);

	if (!bHit)
	{
		if (PreviewActor) PreviewActor->SetActorHiddenInGame(true);
		bHasValidHover = false;
		return;
	}

	bool bCanDeploy = Grid->CanDeployAt(Col, Row) && TowerToDeploy && GM->Cost >= TowerToDeploy.GetDefaultObject()->CostToDeploy;

	// 计算鼠标在格子上的方向
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
	if (!TowerToDeploy) return;

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager) return;

	UTDGridManager* Grid = GM->GridManager;

	if (!bHasValidHover) return;
	if (!Grid->CanDeployAt(HoveredCol, HoveredRow)) return;

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
