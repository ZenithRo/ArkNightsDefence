#include "Core/TDPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Core/TDGameMode.h"
#include "Tower/TDBaseTower.h"
#include "Grid/TDGridManager.h"
#include "Grid/TDGridEnums.h"
#include "UI/TDHandPanel.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
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

	if (GetWorldSettings())
	{
		GetWorldSettings()->SetTimeDilation(1.0f);
	}

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		HandCardLevels.Init(1, GM->LevelHandCards.Num());
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

	if (bDragging)
	{
		UpdateGhostToMouse();
	}

	if (PendingDeployCountdown > 0)
	{
		if (--PendingDeployCountdown == 0)
		{
			int32 Col, Row;
			if (GetMouseGridPosition(Col, Row))
			{
				DeployAtCell(Col, Row);
			}
			else
			{
				CancelPlacement();
			}
		}
	}
}

bool ATDPlayerController::GetMouseGridPosition(int32& OutCol, int32& OutRow) const
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP || !LP->ViewportClient || !LP->ViewportClient->Viewport) return false;

	FViewport* VP = LP->ViewportClient->Viewport;
	FVector2D MouseScreen(VP->GetMouseX(), VP->GetMouseY());

	FVector WorldOrigin, WorldDir;
	if (!DeprojectScreenPositionToWorld(MouseScreen.X, MouseScreen.Y, WorldOrigin, WorldDir)) return false;

	FHitResult Hit;
	FCollisionQueryParams QP;
	QP.bTraceComplex = false;
	FVector EndTrace = WorldOrigin + WorldDir * 50000.0f;
	FVector WorldPos;
	if (GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, EndTrace, TDGridChannels::DeploymentPlane, QP))
	{
		WorldPos = Hit.Location;
	}
	else
	{
		float t = -WorldOrigin.Z / WorldDir.Z;
		if (t <= 0.0f) return false;
		WorldPos = WorldOrigin + WorldDir * t;
	}

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager) return false;

	return GM->GridManager->WorldToGrid(WorldPos, OutCol, OutRow);
}

void ATDPlayerController::UpdateGhostToMouse()
{
	if (!GhostActor) return;

	int32 Col, Row;
	if (!GetMouseGridPosition(Col, Row))
	{
		GhostActor->SetActorHiddenInGame(true);
		return;
	}

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager) return;

	UTDGridManager* Grid = GM->GridManager;

	float SurfaceZ = Grid->GetTileType(Col, Row) == ETileType::HIGHLAND ? 110.0f : 0.0f;
	FVector GridCenter = Grid->GridToWorld(Col, Row);

	GhostActor->SetActorHiddenInGame(false);
	GhostActor->SetActorLocation(FVector(GridCenter.X, GridCenter.Y, SurfaceZ));

	DragCol = Col;
	DragRow = Row;
}

// ---- 手牌接口 ----

int32 ATDPlayerController::GetHandCardCount() const
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	return GM ? GM->LevelHandCards.Num() : 0;
}

TSubclassOf<ATDBaseTower> ATDPlayerController::GetHandCardClass(int32 Index) const
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->LevelHandCards.IsValidIndex(Index)) return nullptr;
	return GM->LevelHandCards[Index];
}

int32 ATDPlayerController::GetHandCardLevel(int32 Index) const
{
	if (!HandCardLevels.IsValidIndex(Index)) return 1;
	return HandCardLevels[Index];
}

float ATDPlayerController::GetHandCardCost(int32 Index) const
{
	TSubclassOf<ATDBaseTower> TowerClass = GetHandCardClass(Index);
	if (!TowerClass) return 0.0f;
	ATDBaseTower* CDO = TowerClass.GetDefaultObject();
	int32 Lvl = GetHandCardLevel(Index);
	int32 Idx = Lvl - 1;
	if (CDO->LevelStats.IsValidIndex(Idx))
	{
		return CDO->LevelStats[Idx].CostToDeploy;
	}
	if (CDO->LevelStats.IsValidIndex(0))
	{
		return CDO->LevelStats[0].CostToDeploy;
	}
	return CDO->CostToDeploy;
}

TArray<int32> ATDPlayerController::GetSortedHandCardIndices() const
{
	TArray<int32> Result;
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return Result;

	for (int32 i = 0; i < GM->LevelHandCards.Num(); i++)
	{
		if (GM->LevelHandCards[i])
		{
			Result.Add(i);
		}
	}
	Result.Sort([this, GM](int32 A, int32 B) {
		auto* CDO_A = GM->LevelHandCards[A].GetDefaultObject();
		auto* CDO_B = GM->LevelHandCards[B].GetDefaultObject();
		float CostA = CDO_A->LevelStats.IsValidIndex(0) ? CDO_A->LevelStats[0].CostToDeploy : CDO_A->CostToDeploy;
		float CostB = CDO_B->LevelStats.IsValidIndex(0) ? CDO_B->LevelStats[0].CostToDeploy : CDO_B->CostToDeploy;
		return CostA < CostB;
	});
	return Result;
}

TArray<int32> ATDPlayerController::GetDescendingSortedHandCardIndices() const
{
	TArray<int32> Result;
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return Result;

	for (int32 i = 0; i < GM->LevelHandCards.Num(); i++)
	{
		if (GM->LevelHandCards[i])
		{
			Result.Add(i);
		}
	}
	Result.Sort([this, GM](int32 A, int32 B) {
		auto* CDO_A = GM->LevelHandCards[A].GetDefaultObject();
		auto* CDO_B = GM->LevelHandCards[B].GetDefaultObject();
		float CostA = CDO_A->LevelStats.IsValidIndex(0) ? CDO_A->LevelStats[0].CostToDeploy : CDO_A->CostToDeploy;
		float CostB = CDO_B->LevelStats.IsValidIndex(0) ? CDO_B->LevelStats[0].CostToDeploy : CDO_B->CostToDeploy;
		return CostA > CostB;
	});
	return Result;
}

void ATDPlayerController::BeginPlacement(int32 Index)
{
	CancelPlacement();

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->LevelHandCards.IsValidIndex(Index)) return;

	PendingTowerClass = GM->LevelHandCards[Index];
	HandCardIndex = Index;
	if (!PendingTowerClass) return;

	ATDBaseTower* DefaultTower = PendingTowerClass.GetDefaultObject();
	if (!DefaultTower) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	GhostActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, SpawnParams);

	if (GhostActor)
	{
		GhostActor->SetActorHiddenInGame(true);

		USceneComponent* Root = NewObject<USceneComponent>(GhostActor, TEXT("Root"));
		GhostActor->SetRootComponent(Root);
		Root->RegisterComponent();

		UStaticMeshComponent* GhostMesh = NewObject<UStaticMeshComponent>(GhostActor);
		if (DefaultTower->TowerMesh && DefaultTower->TowerMesh->GetStaticMesh())
		{
			GhostMesh->SetStaticMesh(DefaultTower->TowerMesh->GetStaticMesh());
		}
		GhostMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GhostMesh->SetupAttachment(Root);
		GhostMesh->RegisterComponent();
	}

	DragCol = -1;
	DragRow = -1;
	bDragging = true;

	UpdateGhostToMouse();
}

void ATDPlayerController::EndPlacement()
{
	if (!bDragging) return;
	bDragging = false;

	if (DragCol >= 0 && DragRow >= 0)
	{
		PendingDeployCountdown = 5;
	}
	else
	{
		CancelPlacement();
	}
}

void ATDPlayerController::DeployAtCell(int32 Col, int32 Row)
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GridManager || !PendingTowerClass)
	{
		CancelPlacement();
		return;
	}

	UTDGridManager* Grid = GM->GridManager;

	ATDBaseTower* CDO = PendingTowerClass.GetDefaultObject();
	if (!CDO)
	{
		CancelPlacement();
		return;
	}

	float DeployCost = GetHandCardCost(HandCardIndex);

	if (!Grid->CanDeployAtWithPlacement(Col, Row, CDO->PlacementType))
	{
		CancelPlacement();
		return;
	}

	if (!GM->SpendCost(DeployCost))
	{
		CancelPlacement();
		return;
	}

	FVector GridCenter = Grid->GridToWorld(Col, Row);
	float SurfaceZ = Grid->GetTileType(Col, Row) == ETileType::HIGHLAND ? 110.0f : 0.0f;
	FVector DeployLoc = FVector(GridCenter.X, GridCenter.Y, SurfaceZ);

	Grid->TryOccupy(Col, Row);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATDBaseTower* Tower = GetWorld()->SpawnActor<ATDBaseTower>(PendingTowerClass, DeployLoc, FRotator::ZeroRotator, SpawnParams);

	if (Tower)
	{
		Tower->SetGridCoordinate(Col, Row);

		// 按当前手牌等级直接设置塔的等级(跳过升级费用)
		int32 CurrentLevel = GetHandCardLevel(HandCardIndex);
		if (CurrentLevel > 1)
		{
			Tower->SetLevelDirectly(CurrentLevel);
		}
	}

	CancelPlacement();
}

void ATDPlayerController::OnTowerUpgraded(TSubclassOf<ATDBaseTower> TowerClass)
{
	if (!TowerClass) return;

	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	// 找到对应的手牌索引
	for (int32 i = 0; i < GM->LevelHandCards.Num(); i++)
	{
		if (GM->LevelHandCards[i] == TowerClass)
		{
			if (HandCardLevels.IsValidIndex(i) && HandCardLevels[i] < 3)
			{
				HandCardLevels[i]++;
				RefreshHandCards();
			}
			break;
		}
	}
}

void ATDPlayerController::RefreshHandCards()
{
	// 通过 GameMode 找到 HandPanel 并刷新
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (GM && GM->HandPanel)
	{
		GM->HandPanel->RefreshAllCards();
	}
}

void ATDPlayerController::OnClick(const FInputActionValue& Value)
{
	if (bDragging)
	{
		CancelPlacement();
	}
}

void ATDPlayerController::CancelPlacement()
{
	bDragging = false;
	DragCol = -1;
	DragRow = -1;
	PendingDeployCountdown = 0;
	HandCardIndex = -1;
	PendingTowerClass = nullptr;
	if (GhostActor)
	{
		GhostActor->Destroy();
		GhostActor = nullptr;
	}
}
