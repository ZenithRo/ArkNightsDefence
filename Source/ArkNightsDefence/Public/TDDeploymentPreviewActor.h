#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDDeploymentPreviewActor.generated.h"

class UStaticMeshComponent;

UCLASS()
class ARKNIGHTSDEFENCE_API ATDDeploymentPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ATDDeploymentPreviewActor();

	void SetValid(bool bValid);

	void SetWorldLocationAndGrid(FVector WorldLoc, int32 Col, int32 Row);

	bool IsLocationValid() const { return bIsValid; }
	int32 GetGridCol() const { return GridCol; }
	int32 GetGridRow() const { return GridRow; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PreviewMesh;

private:
	bool bIsValid = false;
	int32 GridCol = -1;
	int32 GridRow = -1;

	void UpdateColor();
};
