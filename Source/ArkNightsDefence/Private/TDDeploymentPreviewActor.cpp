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

	// 默认使用立方体, 可通过蓝图替换
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		PreviewMesh->SetStaticMesh(CubeMesh.Object);
	}

	// 初始半透明颜色 (绿色)
	PreviewMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), 0.4f);
	PreviewMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FLinearColor::Green);
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
	if (bIsValid)
	{
		PreviewMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FLinearColor::Green);
	}
	else
	{
		PreviewMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FLinearColor::Red);
	}
}
