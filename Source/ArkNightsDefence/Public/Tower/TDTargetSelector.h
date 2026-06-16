#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TDTargetSelector.generated.h"

class ATDEnemy;
class ATDBaseTower;

UENUM(BlueprintType)
enum class ETargetPriority : uint8
{
	NEAREST			UMETA(DisplayName = "最近"),
	FARTHEST		UMETA(DisplayName = "最远"),
	LOWEST_HP		UMETA(DisplayName = "HP最低"),
	HIGHEST_HP		UMETA(DisplayName = "HP最高"),
	LOWEST_DEF		UMETA(DisplayName = "防御最低"),
	HIGHEST_WEIGHT	UMETA(DisplayName = "重量最高"),
	FIRST_BLOCKED	UMETA(DisplayName = "最先被阻挡")
};

UCLASS(Blueprintable)
class ARKNIGHTSDEFENCE_API UTDGTargetSelector : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	ETargetPriority Priority = ETargetPriority::NEAREST;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	int32 MaxTargetCount = 1;

	UFUNCTION(BlueprintCallable, Category = "Targeting")
	TArray<ATDEnemy*> SelectTargets(const TArray<ATDEnemy*>& Candidates, ATDBaseTower* Selector);
};
