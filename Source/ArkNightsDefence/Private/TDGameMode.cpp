#include "TDGameMode.h"
#include "TimerManager.h"

ATDGameMode::ATDGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	Cost = 0.0f;
	Experience = 0;
}

void ATDGameMode::BeginPlay()
{
	Super::BeginPlay();

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
			TEXT("TDGameMode BeginPlay: Cost regen timer started (0.5/sec)"));
	}
}

void ATDGameMode::RegenerateCost()
{
	Cost = FMath::Min(Cost + CostRegenRate, MaxCost);
}

void ATDGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	static int32 TickCount = 0;
	TickCount++;
	if (TickCount <= 3 && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White,
			FString::Printf(TEXT("TDGameMode Tick #%d: Cost=%.1f"), TickCount, Cost));
	}
}

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
}

void ATDGameMode::AddExperience(int32 Amount)
{
	Experience += Amount;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan,
			FString::Printf(TEXT("EXP +%d | Total: %d"), Amount, Experience));
	}
}

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
		return true;
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red,
			FString::Printf(TEXT("Not enough cost! Need: %.0f, Have: %.0f"), Amount, Cost));
	}
	return false;
}
