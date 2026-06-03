#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

// 玩家控制器: 注册EnhancedInput映射, 处理鼠标点击
UCLASS()
class ARKNIGHTSDEFENCE_API ATDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

protected:
	// 输入映射上下文 (IMC_TDGameplay)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;

	// 鼠标点击输入动作 (IA_Click)
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ClickAction;

	// 鼠标点击回调: 射线检测 + Debug红球 + 坐标打印
	void OnClick(const FInputActionValue& Value);
};
