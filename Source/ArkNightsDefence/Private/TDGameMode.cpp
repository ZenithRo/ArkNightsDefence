#include "TDGameMode.h"

ATDGameMode::ATDGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	Cost = MaxCost;
	Experience = 0;
}

void ATDGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Cost = FMath::Min(Cost + CostRegenRate * DeltaTime, MaxCost);
}

void ATDGameMode::EnemyReachedEnd(int32 Damage)
{
	PlayerLives -= Damage;

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
}

bool ATDGameMode::SpendCost(float Amount)
{
	if (Cost >= Amount)
	{
		Cost -= Amount;
		return true;
	}
	return false;
}
