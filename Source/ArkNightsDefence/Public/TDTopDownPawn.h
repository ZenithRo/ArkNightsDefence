#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TDTopDownPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

UCLASS()
class ARKNIGHTSDEFENCE_API ATDTopDownPawn : public APawn
{
	GENERATED_BODY()

public:
	ATDTopDownPawn();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ZoomAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float MoveSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CameraPitch = -60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float CloseZoom = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	float FarZoom = 2000.0f;

	void Move(const FInputActionValue& Value);
	void Zoom(const FInputActionValue& Value);

public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
};
