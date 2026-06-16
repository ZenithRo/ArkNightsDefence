#pragma once

#include "CoreMinimal.h"
#include "TDGridEnums.generated.h"

UENUM(BlueprintType)
enum class ETowerPlacement : uint8
{
	GROUND_ONLY		UMETA(DisplayName = "仅地面"),
	HIGHLAND_ONLY	UMETA(DisplayName = "仅高台"),
	ANY				UMETA(DisplayName = "均可")
};
