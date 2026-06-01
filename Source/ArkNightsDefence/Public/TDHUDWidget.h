#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDHUDWidget.generated.h"

// UMG HUD界面: 通过BindWidget绑定TextBlock, 每帧刷新生命/费用/经验
UCLASS()
class ARKNIGHTSDEFENCE_API UTDHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 生命值显示 (BindWidget: 蓝图中的TextBlock必须命名为TextLives)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextLives;

	// 费用显示 (BindWidget: 蓝图中的TextBlock必须命名为TextCost)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextCost;

	// 经验显示 (BindWidget: 蓝图中的TextBlock必须命名为TextExp)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextExp;
};
