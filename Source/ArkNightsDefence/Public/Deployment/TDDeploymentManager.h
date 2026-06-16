#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDDeploymentManager.generated.h"

UCLASS(Blueprintable)
class ARKNIGHTSDEFENCE_API ATDDeploymentManager : public AActor
{
    GENERATED_BODY()

public:
    ATDDeploymentManager();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cost")
    int32 CurrentCost = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Cost")
    int32 MaxCost = 99;

    UPROPERTY(EditDefaultsOnly, Category = "Cost")
    float CostRegenInterval = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Cost")
    int32 CostRegenAmount = 1;

    UFUNCTION(BlueprintCallable, Category = "Deployment")
    void StartCostRegen();
};
