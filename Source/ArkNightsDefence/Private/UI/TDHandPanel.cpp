#include "UI/TDHandPanel.h"
#include "UI/TDHandCard.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Core/TDPlayerController.h"
#include "Tower/TDBaseTower.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPath.h"
#include "Blueprint/WidgetTree.h"

void UTDHandPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (!RootCanvas) return;
	if (!HandCardClass) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	ATDPlayerController* TDPC = Cast<ATDPlayerController>(PC);
	if (!TDPC) return;

	TArray<int32> SortedIndices = TDPC->GetDescendingSortedHandCardIndices();
	if (SortedIndices.Num() == 0) return;

	const int32 CardSize = 180;
	const int32 Gap = 1;

	for (int32 i = 0; i < SortedIndices.Num(); i++)
	{
		int32 CardIdx = SortedIndices[i];

		UTDHandCard* Card = CreateWidget<UTDHandCard>(PC, HandCardClass);
		if (!Card) continue;

		Card->CardIndex = CardIdx;
		Card->CardCost = TDPC->GetHandCardCost(CardIdx);

		TSubclassOf<ATDBaseTower> TowerSubclass = TDPC->GetHandCardClass(CardIdx);
		if (TowerSubclass)
		{
			ATDBaseTower* DefaultTower = TowerSubclass.GetDefaultObject();
			UEnum* ClassEnum = StaticEnum<ETowerClass>();
			if (ClassEnum)
			{
				FString IconNameStr = ClassEnum->GetDisplayNameTextByValue(static_cast<int64>(DefaultTower->TowerClass)).ToString();
				Card->CardIconName = FText::FromString(IconNameStr);

				FString IconPath = FString::Printf(TEXT("/Game/Resource/ico/%s.%s"), *IconNameStr, *IconNameStr);
				Card->CardIcon = Cast<UTexture2D>(FSoftObjectPath(IconPath).TryLoad());
			}

			FString ClassName = TowerSubclass->GetName();
			ClassName.RemoveFromEnd(TEXT("_C"));
			FString AvatarPath = FString::Printf(TEXT("/Game/Resource/Avatar/%s_avatar.%s_avatar"), *ClassName, *ClassName);
			Card->CardAvatar = Cast<UTexture2D>(FSoftObjectPath(AvatarPath).TryLoad());
		}

		Card->Initialize();

		UCanvasPanelSlot* CardSlot = RootCanvas->AddChildToCanvas(Card);
		if (CardSlot)
		{
			CardSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
			CardSlot->SetAlignment(FVector2D(1.0f, 1.0f));
			CardSlot->SetPosition(FVector2D(-static_cast<float>(i * (CardSize + Gap)), 0.0f));
			CardSlot->SetSize(FVector2D(CardSize, CardSize));
		}
	}
}

void UTDHandPanel::RefreshAllCards()
{
	if (!RootCanvas) return;

	APlayerController* PC = GetOwningPlayer();
	ATDPlayerController* TDPC = Cast<ATDPlayerController>(PC);
	if (!TDPC) return;

	// 清除所有子Widget, 重新创建以更新排序和费用
	RootCanvas->ClearChildren();

	TArray<int32> SortedIndices = TDPC->GetDescendingSortedHandCardIndices();
	if (SortedIndices.Num() == 0) return;

	const int32 CardSize = 180;
	const int32 Gap = 1;

	for (int32 i = 0; i < SortedIndices.Num(); i++)
	{
		int32 CardIdx = SortedIndices[i];

		UTDHandCard* Card = CreateWidget<UTDHandCard>(PC, HandCardClass);
		if (!Card) continue;

		Card->CardIndex = CardIdx;
		Card->CardCost = TDPC->GetHandCardCost(CardIdx);

		TSubclassOf<ATDBaseTower> TowerSubclass = TDPC->GetHandCardClass(CardIdx);
		if (TowerSubclass)
		{
			ATDBaseTower* DefaultTower = TowerSubclass.GetDefaultObject();
			UEnum* ClassEnum = StaticEnum<ETowerClass>();
			if (ClassEnum)
			{
				FString IconNameStr = ClassEnum->GetDisplayNameTextByValue(static_cast<int64>(DefaultTower->TowerClass)).ToString();
				Card->CardIconName = FText::FromString(IconNameStr);

				FString IconPath = FString::Printf(TEXT("/Game/Resource/ico/%s.%s"), *IconNameStr, *IconNameStr);
				Card->CardIcon = Cast<UTexture2D>(FSoftObjectPath(IconPath).TryLoad());
			}

			FString ClassName = TowerSubclass->GetName();
			ClassName.RemoveFromEnd(TEXT("_C"));
			FString AvatarPath = FString::Printf(TEXT("/Game/Resource/Avatar/%s_avatar.%s_avatar"), *ClassName, *ClassName);
			Card->CardAvatar = Cast<UTexture2D>(FSoftObjectPath(AvatarPath).TryLoad());
		}

		Card->Initialize();

		UCanvasPanelSlot* CardSlot = RootCanvas->AddChildToCanvas(Card);
		if (CardSlot)
		{
			CardSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
			CardSlot->SetAlignment(FVector2D(1.0f, 1.0f));
			CardSlot->SetPosition(FVector2D(-static_cast<float>(i * (CardSize + Gap)), 0.0f));
			CardSlot->SetSize(FVector2D(CardSize, CardSize));
		}
	}
}
