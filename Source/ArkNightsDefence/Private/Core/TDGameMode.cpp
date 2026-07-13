#include "Core/TDGameMode.h"
#include "UI/TDHUDWidget.h"
#include "UI/TDHandPanel.h"
#include "Grid/TDGridManager.h"
#include "Grid/TDGridDataActor.h"
#include "Enemy/TDWaveManager.h"
#include "TimerManager.h"
#include "EngineUtils.h"

ATDGameMode::ATDGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	Cost = 0.0f;
	Experience = 0;
}

void ATDGameMode::BeginPlay()
{
	Super::BeginPlay();

	GridManager = NewObject<UTDGridManager>(this);
	GridManager->Initialize(10, 8, 200.0f, FVector::ZeroVector);

	for (TActorIterator<ATDGridDataActor> It(GetWorld()); It; ++It)
	{
		(*It)->ApplyToGridManager(GridManager);
		break;
	}

	GetWorldTimerManager().SetTimer(
		CostRegenTimerHandle,
		this,
		&ATDGameMode::RegenerateCost,
		1.0f,
		true
	);

	if (HandPanelClass && GetWorld())
	{
		HandPanel = CreateWidget<UTDHandPanel>(GetWorld(), HandPanelClass);
		if (HandPanel)
		{
			HandPanel->AddToViewport();
		}
	}

	// 自动查找关卡中的 WaveManager 并启动波次
	if (!WaveManager)
	{
		for (TActorIterator<ATDWaveManager> It(GetWorld()); It; ++It)
		{
			WaveManager = *It;
			break;
		}
	}
	if (WaveManager)
	{
		WaveManager->StartAllWaves();
	}
}

void ATDGameMode::RegenerateCost()
{
	Cost = FMath::Min(Cost + CostRegenRate, MaxCost);
	if (HUDWidget) HUDWidget->UpdateDisplay();
}

void ATDGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (WaveManager)
	{
		int32 NewKilled = WaveManager->GetKilledCount();
		int32 NewTotal = WaveManager->GetTotalCount();
		int32 NewWave = WaveManager->GetCurrentWaveIndex();
		if (NewKilled != WaveKilledCount || NewTotal != WaveTotalCount || NewWave != CurrentWaveIndex)
		{
			WaveKilledCount = NewKilled;
			WaveTotalCount = NewTotal;
			CurrentWaveIndex = NewWave;
			if (HUDWidget) HUDWidget->UpdateDisplay();
		}
	}
}

void ATDGameMode::EnemyReachedEnd(int32 Damage)
{
	PlayerLives -= Damage;

	if (PlayerLives <= 0)
	{
		PlayerLives = 0;
	}

	if (HUDWidget) HUDWidget->UpdateDisplay();
}

void ATDGameMode::AddExperience(int32 Amount)
{
	Experience += Amount;
	if (HUDWidget) HUDWidget->UpdateDisplay();
}

bool ATDGameMode::SpendCost(float Amount)
{
	if (Cost >= Amount)
	{
		Cost -= Amount;
		if (HUDWidget) HUDWidget->UpdateDisplay();
		return true;
	}
	return false;
}

bool ATDGameMode::SpendExperience(int32 Amount)
{
	if (Experience >= Amount)
	{
		Experience -= Amount;
		if (HUDWidget) HUDWidget->UpdateDisplay();
		return true;
	}
	return false;
}
