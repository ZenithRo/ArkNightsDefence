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

	if (!RootCanvas)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HandPanel: RootCanvas is NULL. Name the CanvasPanel 'RootCanvas'."));
		return;
	}

	if (!HandCardClass)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HandPanel: HandCardClass not set!"));
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HandPanel: No OwningPlayer!"));
		return;
	}

	ATDPlayerController* TDPC = Cast<ATDPlayerController>(PC);
	if (!TDPC)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HandPanel: OwningPlayer is not TDPlayerController!"));
		return;
	}

	TArray<int32> SortedIndices = TDPC->GetDescendingSortedHandCardIndices();
	if (SortedIndices.Num() == 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("HandPanel: No hand cards configured in PlayerController!"));
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
			FString::Printf(TEXT("HandPanel: Creating %d cards"), SortedIndices.Num()));
	}

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

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
				FString::Printf(TEXT("  Card[%d]: idx=%d offset=(-%d, 0)"), i, CardIdx, i * (CardSize + Gap)));
		}
	}
}

void UTDHandPanel::RefreshAllCards()
{
	if (!RootCanvas) return;

	APlayerController* PC = GetOwningPlayer();
	ATDPlayerController* TDPC = Cast<ATDPlayerController>(PC);
	if (!TDPC) return;

	TArray<UWidget*> Children = RootCanvas->GetAllChildren();
	for (UWidget* Child : Children)
	{
		UTDHandCard* Card = Cast<UTDHandCard>(Child);
		if (Card)
		{
			float NewCost = TDPC->GetHandCardCost(Card->CardIndex);
			Card->UpdateCost(NewCost);
		}
	}
}
