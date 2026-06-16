#include "TDDeploymentPreviewActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ATDDeploymentPreviewActor::ATDDeploymentPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	PreviewMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	RootComponent = PreviewMesh;
	PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		PreviewMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ATDDeploymentPreviewActor::SetWorldLocationAndGrid(FVector WorldLoc, int32 Col, int32 Row)
{
	SetActorLocation(WorldLoc);
	GridCol = Col;
	GridRow = Row;
}

void ATDDeploymentPreviewActor::SetValid(bool bValid)
{
	bIsValid = bValid;
	UpdateColor();
}

void ATDDeploymentPreviewActor::UpdateColor()
{
	FVector ColorVec = bIsValid ? FVector(0.0f, 1.0f, 0.0f) : FVector(1.0f, 0.0f, 0.0f);

	TArray<UMaterialInterface*> Mats = PreviewMesh->GetMaterials();
	for (int32 i = 0; i < Mats.Num(); i++)
	{
		UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Mats[i]);
		if (!MID)
		{
			MID = PreviewMesh->CreateAndSetMaterialInstanceDynamic(i);
		}
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), ColorVec);
			MID->SetScalarParameterValue(TEXT("Opacity"), 0.4f);
		}
	}
}
