#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Notifications/SProgressBar.h"

#include "TDHealthBarWidget.generated.h"

UCLASS()
class ARKNIGHTSDEFENCE_API UTDHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void SetHealthPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void SetBarColor(const FLinearColor& Color);

private:
	TOptional<float> GetPercent() const;

	TSharedPtr<SProgressBar> MyProgressBar;
	FLinearColor DesiredColor = FLinearColor::Red;
	float CurrentPercent = 1.0f;
};
