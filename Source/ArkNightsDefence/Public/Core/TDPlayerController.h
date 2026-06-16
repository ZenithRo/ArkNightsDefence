#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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

	UPROPERTY(EditDefaultsOnly, Category = "Tower")
	TSubclassOf<ATDDeploymentPreviewActor> PreviewActorClass;

	void OnClick(const FInputActionValue& Value);

private:
	void UpdatePreview();

	TObjectPtr<ATDDeploymentPreviewActor> PreviewActor;

	int32 HoveredCol = -1;
	int32 HoveredRow = -1;
	bool bHasValidHover = false;
};
