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
	// GameMode 保存一局游戏的全局资源，具体初始值也可在蓝图子类中调整。
	PrimaryActorTick.bCanEverTick = true;
	Cost = 0.0f;
	Experience = 0;
}

void ATDGameMode::BeginPlay()
{
	Super::BeginPlay();
	GameEndResult = ETDGameEndResult::InProgress;

	// 运行时创建网格管理器，再读取关卡中的网格数据 Actor。
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

	// 失败优先于胜利：如果最后一个敌人同时使生命归零，应结算为失败。
	if (GameEndResult == ETDGameEndResult::InProgress && PlayerLives <= 0)
	{
		FinishGame(ETDGameEndResult::Defeat);
	}

	// 仅在波次统计发生变化时刷新 HUD，避免 UI 每帧重复构造文本。
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

		// WaveManager 在最后一波的最后一个敌人被击杀/到达终点后标记完成。
		if (GameEndResult == ETDGameEndResult::InProgress &&
			PlayerLives > 0 && WaveManager->AreAllWavesCompleted())
		{
			FinishGame(ETDGameEndResult::Victory);
		}
	}
}

void ATDGameMode::EnemyReachedEnd(int32 Damage)
{
	if (GameEndResult != ETDGameEndResult::InProgress)
	{
		return;
	}

	PlayerLives -= Damage;

	if (PlayerLives <= 0)
	{
		PlayerLives = 0;
		FinishGame(ETDGameEndResult::Defeat);
	}

	if (HUDWidget) HUDWidget->UpdateDisplay();
}

bool ATDGameMode::IsGameOver() const
{
	return GameEndResult != ETDGameEndResult::InProgress;
}

bool ATDGameMode::IsVictory() const
{
	return GameEndResult == ETDGameEndResult::Victory;
}

bool ATDGameMode::IsDefeat() const
{
	return GameEndResult == ETDGameEndResult::Defeat;
}

ETDGameEndResult ATDGameMode::GetGameEndResult() const
{
	return GameEndResult;
}

void ATDGameMode::FinishGame(ETDGameEndResult Result)
{
	if (GameEndResult != ETDGameEndResult::InProgress)
	{
		return;
	}

	GameEndResult = Result;

	// 结算后停止继续生成敌人，避免结算界面出现新的战斗状态。
	if (WaveManager)
	{
		WaveManager->StopAllWaves();
	}

	if (HUDWidget)
	{
		HUDWidget->UpdateDisplay();
	}
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
