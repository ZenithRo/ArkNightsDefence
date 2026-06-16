#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TDHUDWidget.generated.h"

// UMG HUD界面: 由GameMode数据变化时触发更新, 不依赖Tick轮询
UCLASS()
class ARKNIGHTSDEFENCE_API UTDHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 从GameMode读取最新数据并刷新所有TextBlock
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateDisplay();

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
