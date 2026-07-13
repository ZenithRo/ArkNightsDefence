#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDHandCard.generated.h"

class UTexture2D;
class UImage;
class UTextBlock;
class UWidgetAnimation;

UCLASS()
class ARKNIGHTSDEFENCE_API UTDHandCard : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "HandCard")
	void UpdateCost(float NewCost);

	UFUNCTION(BlueprintPure, Category = "HandCard")
	int32 GetCardIndex() const { return CardIndex; }

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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> CostText;

	// 升级动画 (在 WBP_HandCard 蓝图中绑定)
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Upgrade")
	TObjectPtr<UWidgetAnimation> AnimUpdate;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Upgrade")
	TObjectPtr<UWidgetAnimation> AnimSuccess;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Upgrade")
	TObjectPtr<UWidgetAnimation> AnimFalse;

private:
	UFUNCTION()
	void OnCardPressedInternal();

	UFUNCTION()
	void OnCardReleasedInternal();

	void PlayResultAnim(int32 Result);
	void UpdateUpgradeAnim();

	bool bWasInUpgradeMode = false;
	FTimerHandle UpgradeCheckTimer;
};