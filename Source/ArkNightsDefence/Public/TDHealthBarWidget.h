#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDHealthBarWidget.generated.h"

class UProgressBar;
class UCanvasPanel;

// 纯C++血条Widget: CanvasPanel根+ProgressBar子, 标准UMG布局
UCLASS()
class ARKNIGHTSDEFENCE_API UTDHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPercent(float NewPercent);
	void SetBarColor(const FLinearColor& NewColor);
	void UpdateBar();

	float GetPercent() const { return Percent; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;

	float Percent = 1.0f;
	FLinearColor BarColor = FLinearColor::Red;
};
