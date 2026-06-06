#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ATDBaseTower;
struct FInputActionValue;

// 玩家控制器: 注册EnhancedInput映射, 鼠标点击部署塔
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

	// 当前要部署的塔蓝图类 (在BP_TDPlayerController中指定)
	UPROPERTY(EditDefaultsOnly, Category = "Tower")
	TSubclassOf<ATDBaseTower> TowerToDeploy;

	// 鼠标点击回调: 扣费 + 生成塔
	void OnClick(const FInputActionValue& Value);
};
