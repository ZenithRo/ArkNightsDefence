#pragma once

#include "CoreMinimal.h"
#include "TDAttackRange.generated.h"

// 攻击范围格子: 相对塔格子坐标的偏移
USTRUCT(BlueprintType)
struct FAttackRangeCell
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AttackRange")
	int32 DeltaX = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AttackRange")
	int32 DeltaY = 0;
};
