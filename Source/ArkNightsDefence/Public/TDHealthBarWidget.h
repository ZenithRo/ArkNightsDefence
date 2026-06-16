#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "TDHealthBarWidget.generated.h"

UCLASS()
class ARKNIGHTSDEFENCE_API UTDHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void SetHealthPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category = "HealthBar")
	void SetBarColor(const FLinearColor& Color);

private:
	TObjectPtr<UProgressBar> HealthBar;
};
