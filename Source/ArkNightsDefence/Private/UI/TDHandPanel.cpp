#include "UI/TDHandPanel.h"
#include "UI/TDHandCard.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Core/TDPlayerController.h"
#include "Tower/TDBaseTower.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPath.h"

DEFINE_LOG_CATEGORY_STATIC(LogHandPanel, Log, All);

void UTDHandPanel::NativeConstruct()
{
	Super::NativeConstruct();

	UE_LOG(LogHandPanel, Log, TEXT("[1] NativeConstruct entered"));

	if (!RootCanvas)
	{
		UE_LOG(LogHandPanel, Error, TEXT("[X] RootCanvas is NULL"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HandPanel: RootCanvas is NULL"));
		return;
	}
	UE_LOG(LogHandPanel, Log, TEXT("[2] RootCanvas OK"));

	if (!HandCardClass)
	{
		UE_LOG(LogHandPanel, Error, TEXT("[X] HandCardClass not set"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HandPanel: HandCardClass not set!"));
		return;
	}
	UE_LOG(LogHandPanel, Log, TEXT("[3] HandCardClass OK"));

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogHandPanel, Error, TEXT("[X] No OwningPlayer"));
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("HandPanel: No OwningPlayer!"));
		return;
	}
	UE_LOG(LogHandPanel, Log, TEXT("[4] OwningPlayer OK"));

	ATDPlayerController* TDPC = Cast<ATDPlayerController>(PC);
	if (!TDPC)
	{
		UE_LOG(LogHandPanel, Error, TEXT("[X] PC is not TDPlayerController"));
		return;
	}
	UE_LOG(LogHandPanel, Log, TEXT("[5] TDPlayerController OK"));

	TArray<int32> SortedIndices = TDPC->GetDescendingSortedHandCardIndices();
	if (SortedIndices.Num() == 0)
	{
		UE_LOG(LogHandPanel, Warning, TEXT("[X] No hand cards configured"));
		return;
	}
	UE_LOG(LogHandPanel, Log, TEXT("[6] Have %d cards"), SortedIndices.Num());

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
				UE_LOG(LogHandPanel, Log, TEXT("[icon] loading from: %s"), *IconPath);
				Card->CardIcon = Cast<UTexture2D>(FSoftObjectPath(IconPath).TryLoad());
			}

			FString ClassName = TowerSubclass->GetName();
			ClassName.RemoveFromEnd(TEXT("_C"));
			FString AvatarPath = FString::Printf(TEXT("/Game/Resource/Avatar/%s_avatar.%s_avatar"), *ClassName, *ClassName);
			UE_LOG(LogHandPanel, Log, TEXT("[avatar] loading from: %s"), *AvatarPath);
			Card->CardAvatar = Cast<UTexture2D>(FSoftObjectPath(AvatarPath).TryLoad());
		}

		UCanvasPanelSlot* CardSlot = RootCanvas->AddChildToCanvas(Card);
		if (CardSlot)
		{
			CardSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
			CardSlot->SetAlignment(FVector2D(1.0f, 1.0f));
			CardSlot->SetPosition(FVector2D(-static_cast<float>(i * (CardSize + Gap)), 0.0f));
			CardSlot->SetSize(FVector2D(CardSize, CardSize));
		}

		UE_LOG(LogHandPanel, Log, TEXT("[10.%d] Card %d done"), i, CardIdx);
	}

	UE_LOG(LogHandPanel, Log, TEXT("[11] NativeConstruct complete"));
}
