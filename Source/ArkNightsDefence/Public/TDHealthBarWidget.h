#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDHealthBarWidget.generated.h"

class UProgressBar;

// 纯C++血条Widget: 程序化创建ProgressBar, 无需UMG蓝图
UCLASS()
class ARKNIGHTSDEFENCE_API UTDHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 设置血条百分比 (0~1)
	void SetPercent(float NewPercent);
	// 设置血条填充颜色
	void SetBarColor(const FLinearColor& NewColor);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;

	float Percent = 1.0f;
	FLinearColor BarColor = FLinearColor::Red;
};
