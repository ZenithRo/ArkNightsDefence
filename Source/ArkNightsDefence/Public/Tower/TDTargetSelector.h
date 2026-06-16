#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TDTargetSelector.generated.h"

UENUM(BlueprintType)
enum class ETargetPriority : uint8
{
    NEAREST,
    FARTHEST,
    LOWEST_HP,
    HIGHEST_HP,
    LOWEST_DEF,
    HIGHEST_WEIGHT,
    FIRST_BLOCKED
};

UCLASS(Blueprintable)
class ARKNIGHTSDEFENCE_API UTDGAntiSelector : public UObject
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    ETargetPriority Priority = ETargetPriority::NEAREST;
    
    UPROPERTY(EditDefaultsOnly, Category = "Targeting")
    int32 MaxTargetCount = 1;
};
