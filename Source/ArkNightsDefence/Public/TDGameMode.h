#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TDGameMode.generated.h"

UCLASS()
class ARKNIGHTSDEFENCE_API ATDGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATDGameMode();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Lives")
	int32 PlayerLives = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Cost")
	float MaxCost = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Game|Cost")
	float Cost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game|Cost")
	float CostRegenRate = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Game|Experience")
	int32 Experience;

	UFUNCTION(BlueprintCallable, Category = "Game")
	void EnemyReachedEnd(int32 Damage);

	UFUNCTION(BlueprintCallable, Category = "Game")
	void AddExperience(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Game")
	bool SpendCost(float Amount);

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle CostRegenTimerHandle;

	void RegenerateCost();
};
