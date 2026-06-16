#include "Enemy/TDWaveManager.h"
#include "Enemy/TDEnemy.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATDWaveManager::ATDWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATDWaveManager::BeginPlay()
{
	Super::BeginPlay();
}

void ATDWaveManager::StartAllWaves()
{
	if (WaveConfigs.Num() == 0) return;

	CurrentWaveIndex = 0;
	SpawnWave(CurrentWaveIndex);
}

void ATDWaveManager::SpawnWave(int32 WaveIndex)
{
	if (!WaveConfigs.IsValidIndex(WaveIndex)) return;

	const FWaveData& Wave = WaveConfigs[WaveIndex];

	for (const FWaveEnemyEntry& Entry : Wave.Enemies)
	{
		if (!Entry.EnemyClass) continue;

		FTimerHandle SpawnTimer;
		FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &ATDWaveManager::SpawnEnemy, Entry.EnemyClass, Entry.PathIndex);
		GetWorldTimerManager().SetTimer(SpawnTimer, Delegate, Entry.SpawnDelay, false);
	}
}

void ATDWaveManager::SpawnEnemy(TSubclassOf<ATDEnemy> EnemyClass, int32 PathIndex)
{
	if (!EnemyClass || !PathActors.IsValidIndex(PathIndex)) return;

	AActor* PathActor = PathActors[PathIndex];
	if (!PathActor) return;

	USplineComponent* Spline = PathActor->FindComponentByClass<USplineComponent>();
	if (!Spline) return;

	// 在Spline起点生成敌人
	FVector SpawnLoc = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	FRotator SpawnRot = Spline->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATDEnemy* NewEnemy = GetWorld()->SpawnActor<ATDEnemy>(EnemyClass, SpawnLoc, SpawnRot, SpawnParams);
	if (NewEnemy)
	{
		NewEnemy->PathActor = PathActor;
	}
}
