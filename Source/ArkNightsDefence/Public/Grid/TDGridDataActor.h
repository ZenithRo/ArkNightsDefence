#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grid/TDGridManager.h"
#include "TDGridDataActor.generated.h"

UCLASS(Blueprintable)
class ARKNIGHTSDEFENCE_API ATDGridDataActor : public AActor
{
	GENERATED_BODY()

public:
	ATDGridDataActor();

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void ApplyToGridManager(UTDGridManager* Manager) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 NumCols = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 NumRows = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float CellSize = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FVector GridOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TArray<FGridCellData> Cells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float DrawHeight = 0.0f;

protected:
	virtual void BeginPlay() override;

	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	void DrawEditorGrid();
};
