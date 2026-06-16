#include "TDGridManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"

void UTDGridManager::Initialize(int32 InCols, int32 InRows, float InCellSize, FVector InOrigin)
{
	NumCols = InCols;
	NumRows = InRows;
	CellSize = InCellSize;
	GridOrigin = InOrigin;

	Cells.SetNum(NumCols * NumRows);
	for (FGridCellData& Cell : Cells)
	{
		Cell.bDeployable = true;
		Cell.bOccupied = false;
	}
}

int32 UTDGridManager::GetIndex(int32 Col, int32 Row) const
{
	return Row * NumCols + Col;
}

bool UTDGridManager::WorldToGrid(FVector WorldPos, int32& OutCol, int32& OutRow) const
{
	float LocalX = WorldPos.X - GridOrigin.X;
	float LocalY = WorldPos.Y - GridOrigin.Y;

	OutCol = FMath::FloorToInt(LocalX / CellSize);
	OutRow = FMath::FloorToInt(LocalY / CellSize);

	return IsValidCell(OutCol, OutRow);
}

FVector UTDGridManager::GridToWorld(int32 Col, int32 Row) const
{
	float CenterX = GridOrigin.X + (Col + 0.5f) * CellSize;
	float CenterY = GridOrigin.Y + (Row + 0.5f) * CellSize;
	return FVector(CenterX, CenterY, GridOrigin.Z);
}

bool UTDGridManager::IsValidCell(int32 Col, int32 Row) const
{
	return Col >= 0 && Col < NumCols && Row >= 0 && Row < NumRows;
}

bool UTDGridManager::CanDeployAt(int32 Col, int32 Row) const
{
	if (!IsValidCell(Col, Row)) return false;

	const FGridCellData& Cell = Cells[GetIndex(Col, Row)];
	return Cell.bDeployable && !Cell.bOccupied;
}

bool UTDGridManager::TryOccupy(int32 Col, int32 Row)
{
	if (!CanDeployAt(Col, Row)) return false;

	Cells[GetIndex(Col, Row)].bOccupied = true;
	return true;
}

void UTDGridManager::Free(int32 Col, int32 Row)
{
	if (IsValidCell(Col, Row))
	{
		Cells[GetIndex(Col, Row)].bOccupied = false;
	}
}

bool UTDGridManager::GetDeployLocation(const APlayerController* PC, FVector& OutLocation, int32& OutCol, int32& OutRow) const
{
	if (!PC || !PC->GetWorld()) return false;

	// 第一层射线: 从摄像机穿透地图, 命中高空判定平面 (DeploymentPlane Trace Channel)
	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);

	FHitResult PlaneHit;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	FVector EndTrace = CamLoc + CamRot.Vector() * 50000.0f;

	if (!PC->GetWorld()->LineTraceSingleByChannel(PlaneHit, CamLoc, EndTrace, TDGridChannels::DeploymentPlane, QueryParams))
	{
		return false;
	}

	// 坐标换算: 世界坐标 -> 格子坐标
	int32 Col, Row;
	float LocalX = PlaneHit.Location.X - GridOrigin.X;
	float LocalY = PlaneHit.Location.Y - GridOrigin.Y;
	Col = FMath::FloorToInt(LocalX / CellSize);
	Row = FMath::FloorToInt(LocalY / CellSize);

	if (!IsValidCell(Col, Row)) return false;

	OutCol = Col;
	OutRow = Row;

	// 格子中心世界坐标 (X, Y)
	FVector GridCenter = GridToWorld(Col, Row);

	// 第二层射线: 从格子中心高空垂直向下, 检测地形高度 (MapMesh Trace Channel)
	FVector TraceStart = FVector(GridCenter.X, GridCenter.Y, 10000.0f);
	FVector TraceEnd = FVector(GridCenter.X, GridCenter.Y, -1000.0f);

	FHitResult TerrainHit;
	FCollisionQueryParams TerrainParams;
	TerrainParams.bTraceComplex = false;

	if (PC->GetWorld()->LineTraceSingleByChannel(TerrainHit, TraceStart, TraceEnd, TDGridChannels::MapMesh, TerrainParams))
	{
		OutLocation = TerrainHit.Location;
	}
	else
	{
		// 没有命中地形时用平面Z作为fallback
		OutLocation = FVector(GridCenter.X, GridCenter.Y, PlaneHit.Location.Z);
	}

	return true;
}
