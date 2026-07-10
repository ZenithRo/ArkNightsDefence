#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Tower/TDDeployDirection.h"
#include "TDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ATDBaseTower;
class ATDDeploymentPreviewActor;
struct FInputActionValue;

UCLASS()
class ARKNIGHTSDEFENCE_API ATDPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY(EditDefaultsOnly, Category = "Tower")
	TSubclassOf<ATDBaseTower> TowerToDeploy;

	// 手牌中的可用塔列表(可在蓝图编辑器中设置)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower")
	TArray<TSubclassOf<ATDBaseTower>> HandCards;

	// 当前选中的手牌索引, -1表示未选中
	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	int32 SelectedHandCardIndex = -1;

public:
	// 选择手牌中的一张塔(自动取消上一个选中, 置空TowerToDeploy)
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void SelectHandCard(int32 Index);

	// 取消选中
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void DeselectHandCard();

	// 获取手牌数量
	UFUNCTION(BlueprintCallable, Category = "Tower")
	int32 GetHandCardCount() const { return HandCards.Num(); }

	// 获取指定手牌的塔类
	UFUNCTION(BlueprintCallable, Category = "Tower")
	TSubclassOf<ATDBaseTower> GetHandCardClass(int32 Index) const;

	// 获取指定手牌的部署费用
	UFUNCTION(BlueprintCallable, Category = "Tower")
	float GetHandCardCost(int32 Index) const;

	// 按费用从低到高返回排序后的索引数组(用于手牌排列)
	UFUNCTION(BlueprintCallable, Category = "Tower")
	TArray<int32> GetSortedHandCardIndices() const;

	// 按费用从高到低返回排序后的索引数组(右边→左边排列)
	UFUNCTION(BlueprintCallable, Category = "Tower")
	TArray<int32> GetDescendingSortedHandCardIndices() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Tower")
	TSubclassOf<ATDDeploymentPreviewActor> PreviewActorClass;

	void OnClick(const FInputActionValue& Value);

private:
	void UpdatePreview();

	EDeployDirection GetDirectionFromMouse(FVector GridWorldCenter, FVector MouseWorldPos) const;

	TObjectPtr<ATDDeploymentPreviewActor> PreviewActor;

	int32 HoveredCol = -1;
	int32 HoveredRow = -1;
	EDeployDirection HoveredDirection = EDeployDirection::RIGHT;
	bool bHasValidHover = false;
};
