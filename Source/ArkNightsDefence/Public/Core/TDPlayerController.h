#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Tower/TDDeployDirection.h"
#include "TDPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ATDBaseTower;
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

	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	int32 SelectedHandCardIndex = -1;

public:
	UFUNCTION(BlueprintCallable, Category = "Tower")
	int32 GetHandCardCount() const;

	UFUNCTION(BlueprintCallable, Category = "Tower")
	TSubclassOf<ATDBaseTower> GetHandCardClass(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Tower")
	float GetHandCardCost(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Tower")
	int32 GetHandCardLevel(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Tower")
	TArray<int32> GetSortedHandCardIndices() const;

	UFUNCTION(BlueprintCallable, Category = "Tower")
	TArray<int32> GetDescendingSortedHandCardIndices() const;

	// 手牌按钮按下
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void BeginPlacement(int32 Index);

	// 手牌按钮弹起
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void EndPlacement();

	// 塔升级后调用: 提升对应手牌等级, 刷新UI费用
	UFUNCTION(BlueprintCallable, Category = "Tower")
	void OnTowerUpgraded(TSubclassOf<ATDBaseTower> TowerClass);

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void RefreshHandCards();

	void OnClick(const FInputActionValue& Value);

	UFUNCTION()
	void OnPauseToggle();

	UFUNCTION()
	void OnSpeedToggle();

private:
	void UpdateGhostToMouse();
	void DeployAtCell(int32 Col, int32 Row);
	void CancelPlacement();
	bool GetMouseGridPosition(int32& OutCol, int32& OutRow) const;

	class AActor* GhostActor = nullptr;

	// 每个手牌位的当前等级(默认1), Index对应LevelHandCards中的顺序
	UPROPERTY()
	TArray<int32> HandCardLevels;

	int32 HandCardIndex = -1;
	TSubclassOf<ATDBaseTower> PendingTowerClass;

	bool bDragging = false;
	int32 DragCol = -1;
	int32 DragRow = -1;
	int32 PendingDeployCountdown = 0;

	// 暂停/倍速
	float PreviousTimeDilation = 1.0f;
	bool bIsPaused = false;
};
