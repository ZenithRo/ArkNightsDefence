#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/SlateTypes.h"

#include "TDHealthBarWidget.generated.h"

UCLASS()
class ARKNIGHTSDEFENCE_API UTDHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTDHealthBarWidget();

	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void SetHealthPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void SetBarColor(const FLinearColor& Color);

private:
	float GetPercent() const;

	TSharedPtr<SProgressBar> MyProgressBar;
	FProgressBarStyle BarStyle;
	float CurrentPercent = 1.0f;
};
