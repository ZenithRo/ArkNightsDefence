#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDHandCard.generated.h"

class UTexture2D;
class UImage;

UCLASS()
class ARKNIGHTSDEFENCE_API UTDHandCard : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, Category = "HandCard", meta = (ExposeOnSpawn = true))
	int32 CardIndex = -1;

	UPROPERTY(BlueprintReadOnly, Category = "HandCard")
	float CardCost = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "HandCard")
	FText CardIconName;

	UPROPERTY(BlueprintReadOnly, Category = "HandCard")
	TObjectPtr<UTexture2D> CardIcon;

	UPROPERTY(BlueprintReadOnly, Category = "HandCard")
	TObjectPtr<UTexture2D> CardAvatar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> CardIconImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> CardAvatarImage;
};
