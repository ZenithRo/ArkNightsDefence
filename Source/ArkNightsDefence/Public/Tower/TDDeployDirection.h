#pragma once

#include "CoreMinimal.h"
#include "TDDeployDirection.generated.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
	RIGHT	UMETA(DisplayName = "朝右"),
	LEFT	UMETA(DisplayName = "朝左"),
	UP		UMETA(DisplayName = "朝上"),
	DOWN	UMETA(DisplayName = "朝下")
};
