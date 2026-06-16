// GameMode实现: 生命系统, 费用自动恢复, 经验累积, 全局Debug日志
#include "Core/TDGameMode.h"
#include "UI/TDHUDWidget.h"
#include "Grid/TDGridManager.h"
#include "Grid/TDGridDataActor.h"
#include "TimerManager.h"
#include "EngineUtils.h"

ATDGameMode::ATDGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// 初始费用 = 0 (等自动回复慢慢涨), 经验 = 0
	Cost = 0.0f;
	Experience = 0;
}

void ATDGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 创建网格管理器
	GridManager = NewObject<UTDGridManager>(this);
	GridManager->Initialize(10, 8, 200.0f, FVector::ZeroVector);

	// 从关卡中查找TDGridDataActor, 应用其编辑数据
	for (TActorIterator<ATDGridDataActor> It(GetWorld()); It; ++It)
	{
		(*It)->ApplyToGridManager(GridManager);
		break;
	}

	// 启动费用恢复定时器: 每秒+1.0
	GetWorldTimerManager().SetTimer(
		CostRegenTimerHandle,
		this,
		&ATDGameMode::RegenerateCost,
		1.0f,
		true
	);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
			TEXT("TDGameMode BeginPlay: Cost regen timer started (1.0/sec)"));
	}
}

// 费用恢复: 每次+1.0, 不超过上限
void ATDGameMode::RegenerateCost()
{
	Cost = FMath::Min(Cost + CostRegenRate, MaxCost);
	if (HUDWidget) HUDWidget->UpdateDisplay();
}

void ATDGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 前3帧打印Cost调试信息
	static int32 TickCount = 0;
	TickCount++;
	if (TickCount <= 3 && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White,
			FString::Printf(TEXT("TDGameMode Tick #%d: Cost=%.1f"), TickCount, Cost));
	}
}

// 敌人到达终点: 扣减生命, 生命归零时GameOver
void ATDGameMode::EnemyReachedEnd(int32 Damage)
{
	PlayerLives -= Damage;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange,
			FString::Printf(TEXT("Enemy leaked! Lives: %d"), PlayerLives));
	}

	if (PlayerLives <= 0)
	{
		PlayerLives = 0;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("GAME OVER!"));
		}
	}

	if (HUDWidget) HUDWidget->UpdateDisplay();
}

// 获得经验: 击杀敌人时调用
void ATDGameMode::AddExperience(int32 Amount)
{
	Experience += Amount;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
			FString::Printf(TEXT("EXP +%d | Total: %d"), Amount, Experience));
	}

	if (HUDWidget) HUDWidget->UpdateDisplay();
}

// 消耗费用: 用于部署防御塔, 返回是否成功 (费用不足则拒绝)
bool ATDGameMode::SpendCost(float Amount)
{
	if (Cost >= Amount)
	{
		Cost -= Amount;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
				FString::Printf(TEXT("Cost -%.0f | Remaining: %.0f"), Amount, Cost));
		}
		if (HUDWidget) HUDWidget->UpdateDisplay();
		return true;
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
			FString::Printf(TEXT("Not enough cost! Need: %.0f, Have: %.0f"), Amount, Cost));
	}
	return false;
}

// 消耗经验: 用于升级塔, 返回是否成功 (经验不足则拒绝)
bool ATDGameMode::SpendExperience(int32 Amount)
{
	if (Experience >= Amount)
	{
		Experience -= Amount;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow,
				FString::Printf(TEXT("EXP -%d | Remaining: %d"), Amount, Experience));
		}
		if (HUDWidget) HUDWidget->UpdateDisplay();
		return true;
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
			FString::Printf(TEXT("Not enough EXP! Need: %d, Have: %d"), Amount, Experience));
	}
	return false;
}
