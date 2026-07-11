#pragma once

#include "CoreMinimal.h"
#include "TDAttackRange.generated.h"

UENUM(BlueprintType)
enum class EAttackRangeMode : uint8
{
	Matrix		UMETA(DisplayName = "矩阵(格子偏移)"),
	Circle		UMETA(DisplayName = "圆形(半径)")
};

UENUM(BlueprintType)
enum class EAttackTargetType : uint8
{
	Land	UMETA(DisplayName = "仅地面(Land)"),
	Fly		UMETA(DisplayName = "仅飞行(Fly)"),
	Both	UMETA(DisplayName = "均可")
};

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
