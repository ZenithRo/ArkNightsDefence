#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TDTopDownPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

// 俯视角摄像机Pawn: 提供WASD平移 + 滚轮缩放 + 可调俯角
UCLASS()
class ARKNIGHTSDEFENCE_API ATDTopDownPawn : public APawn
{
	GENERATED_BODY()

public:
	ATDTopDownPawn();

protected:
	virtual void BeginPlay() override;

	// 弹簧臂组件: 控制相机距离与碰撞
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	// 摄像机组件: 挂在弹簧臂末端
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	// WASD移动输入动作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	// 滚轮缩放输入动作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ZoomAction;

	// 移动速度 (厘米/秒)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MoveSpeed = 1500.0f;

	// 摄像机俯角 (负值=向下看)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraPitch = -60.0f;

	// 近景缩放臂长
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CloseZoom = 800.0f;

	// 远景缩放臂长
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float FarZoom = 2000.0f;

	// 基于摄像机方向水平移动
	void Move(const FInputActionValue& Value);
	// 滚轮放大/缩小
	void Zoom(const FInputActionValue& Value);

public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
