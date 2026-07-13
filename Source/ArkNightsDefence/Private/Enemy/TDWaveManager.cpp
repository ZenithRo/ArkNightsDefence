#include "Enemy/TDWaveManager.h"
#include "Enemy/TDEnemy.h"
#include "Enemy/TDWaveTypes.h"
#include "Components/SplineComponent.h"
#include "Engine/DataTable.h"
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
	if (!WaveDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ATDWaveManager::StartAllWaves: WaveDataTable is null"));
		return;
	}

	WaveRowNames.Empty();
	for (const FName& RowName : WaveDataTable->GetRowNames())
	{
		if (RowName.ToString().StartsWith(TEXT("Wave_")))
		{
			WaveRowNames.Add(RowName);
		}
	}
	WaveRowNames.Sort([](const FName& A, const FName& B)
	{
		int32 NumA = FCString::Atoi(*A.ToString().RightChop(5));
		int32 NumB = FCString::Atoi(*B.ToString().RightChop(5));
		return NumA < NumB;
	});

	if (WaveRowNames.Num() == 0) return;

	// 计算所有波次的总敌人数
	TotalCount = 0;
	for (const FName& RowName : WaveRowNames)
	{
		FTDWaveTableRow* Row = WaveDataTable->FindRow<FTDWaveTableRow>(RowName, TEXT(""));
		if (Row)
		{
			for (const FTDWaveSpawnEntry& Entry : Row->SpawnEntries)
			{
				TotalCount += Entry.Count;
			}
		}
	}

	CurrentWaveIndex = 0;
	KilledCount = 0;

	StartWave(CurrentWaveIndex);
}

void ATDWaveManager::StartWave(int32 WaveIndex)
{
	if (!WaveRowNames.IsValidIndex(WaveIndex)) return;

	FTDWaveTableRow* Row = WaveDataTable->FindRow<FTDWaveTableRow>(WaveRowNames[WaveIndex], TEXT(""));
	if (!Row) return;

	OnWaveChanged.Broadcast(WaveIndex);

	ActiveSpawnStates.Empty();
	WaveTotalEnemies = 0;
	WaveProcessedEnemies = 0;

	for (const FTDWaveSpawnEntry& Entry : Row->SpawnEntries)
	{
		if (!Entry.EnemyClass) continue;

		FWaveSpawnState State;
		State.EnemyClass = Entry.EnemyClass;
		State.Remaining = Entry.Count;
		State.SpawnInterval = Entry.SpawnInterval;
		State.PathIndex = Entry.PathIndex;
		ActiveSpawnStates.Add(State);
		WaveTotalEnemies += Entry.Count;
	}

	if (ActiveSpawnStates.Num() > 0)
	{
		float Delay = Row->WaveStartDelay;
		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ATDWaveManager::SpawnNextFromState, Delay, false);
	}
}

void ATDWaveManager::SpawnNextFromState()
{
	// 找到一个还有剩余数量的生成状态
	for (FWaveSpawnState& State : ActiveSpawnStates)
	{
		if (State.Remaining > 0)
		{
			State.Remaining--;

			if (!State.EnemyClass || !PathActors.IsValidIndex(State.PathIndex)) return;

			AActor* PathActor = PathActors[State.PathIndex];
			if (!PathActor) return;

			USplineComponent* Spline = PathActor->FindComponentByClass<USplineComponent>();
			if (!Spline) return;

			FVector SpawnLoc = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ATDEnemy* NewEnemy = GetWorld()->SpawnActor<ATDEnemy>(State.EnemyClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
			if (NewEnemy)
			{
				NewEnemy->SetPathActor(PathActor);
				NewEnemy->OnEnemyFinished.AddUObject(this, &ATDWaveManager::OnEnemyKilled);
				NewEnemy->OnEnemyReachedEndDel.AddUObject(this, &ATDWaveManager::OnEnemyReachedEnd);
			}

			// 安排下一个生成
			GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ATDWaveManager::SpawnNextFromState, State.SpawnInterval, false);
			return;
		}
	}
}

void ATDWaveManager::OnEnemyKilled()
{
	KilledCount++;
	WaveProcessedEnemies++;
	OnWaveProgress.Broadcast(KilledCount, TotalCount);
	CheckWaveComplete();
}

void ATDWaveManager::OnEnemyReachedEnd()
{
	WaveProcessedEnemies++;
	CheckWaveComplete();
}

void ATDWaveManager::CheckWaveComplete()
{
	if (WaveProcessedEnemies >= WaveTotalEnemies)
	{
		int32 NextWave = CurrentWaveIndex + 1;
		if (WaveRowNames.IsValidIndex(NextWave))
		{
			CurrentWaveIndex = NextWave;
			StartWave(CurrentWaveIndex);
		}
	}
}
