#include "Tower/TDTargetSelector.h"
#include "Enemy/TDEnemy.h"
#include "Tower/TDBaseTower.h"

TArray<ATDEnemy*> UTDGTargetSelector::SelectTargets(const TArray<ATDEnemy*>& Candidates, ATDBaseTower* Selector)
{
	// 选择器只负责对传入候选集排序；攻击范围和目标类型由防御塔先行过滤。
	TArray<ATDEnemy*> SortedCandidates = Candidates;

	switch (Priority)
	{
	case ETargetPriority::NEAREST:
		SortedCandidates.Sort([Selector](const ATDEnemy& A, const ATDEnemy& B)
		{
			float DistA = FVector::Dist(Selector->GetActorLocation(), A.GetActorLocation());
			float DistB = FVector::Dist(Selector->GetActorLocation(), B.GetActorLocation());
			return DistA < DistB;
		});
		break;

	case ETargetPriority::FARTHEST:
		SortedCandidates.Sort([Selector](const ATDEnemy& A, const ATDEnemy& B)
		{
			float DistA = FVector::Dist(Selector->GetActorLocation(), A.GetActorLocation());
			float DistB = FVector::Dist(Selector->GetActorLocation(), B.GetActorLocation());
			return DistA > DistB;
		});
		break;

	case ETargetPriority::LOWEST_HP:
		SortedCandidates.Sort([](const ATDEnemy& A, const ATDEnemy& B)
		{
			return A.CurrentHealth < B.CurrentHealth;
		});
		break;

	case ETargetPriority::HIGHEST_HP:
		SortedCandidates.Sort([](const ATDEnemy& A, const ATDEnemy& B)
		{
			return A.CurrentHealth > B.CurrentHealth;
		});
		break;

	default:
		break;
	}

	TArray<ATDEnemy*> Result;
	for (int32 i = 0; i < FMath::Min(SortedCandidates.Num(), MaxTargetCount); i++)
	{
		Result.Add(SortedCandidates[i]);
	}

	return Result;
}
