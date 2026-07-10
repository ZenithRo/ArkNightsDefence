#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDHandPanel.generated.h"

UCLASS()
class ARKNIGHTSDEFENCE_API UTDHandPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> RootCanvas;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "HandPanel")
	TSubclassOf<class UUserWidget> HandCardClass;
};
