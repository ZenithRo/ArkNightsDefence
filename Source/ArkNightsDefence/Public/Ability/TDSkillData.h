#pragma once

#include "CoreMinimal.h"
#include "TDSkillData.generated.h"

UENUM(BlueprintType)
enum class ESkillRecoveryType : uint8
{
    AUTO,
    ATTACK,
    HIT
};

UENUM(BlueprintType)
enum class ESkillTriggerType : uint8
{
    MANUAL,
    AUTO
};

USTRUCT(BlueprintType)
struct FSkillData
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill")
    FString SkillName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill")
    ESkillRecoveryType RecoveryType = ESkillRecoveryType::AUTO;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill")
    ESkillTriggerType TriggerType = ESkillTriggerType::MANUAL;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill")
    int32 InitialSP = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill")
    int32 MaxSP = 10;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill")
    float Duration = 0.0f;
};
