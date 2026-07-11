#include "UI/TDHandCard.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"
#include "Core/TDPlayerController.h"

void UTDHandCard::NativeConstruct()
{
	Super::NativeConstruct();

	if (CardIconImage)
	{
		if (CardIcon)
		{
			CardIconImage->SetBrushFromTexture(CardIcon);
		}
		CardIconImage->SetDesiredSizeOverride(FVector2D(35.0f, 35.0f));
	}

	if (CardAvatarImage && CardAvatar)
	{
		CardAvatarImage->SetBrushFromTexture(CardAvatar);
		CardAvatarImage->SetDesiredSizeOverride(FVector2D(180.0f, 180.0f));
	}

	UpdateCost(CardCost);

	UButton* Btn = nullptr;
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);
	for (UWidget* W : Widgets)
	{
		Btn = Cast<UButton>(W);
		if (Btn) break;
	}

	if (Btn)
	{
		Btn->OnPressed.Clear();
		Btn->OnPressed.AddDynamic(this, &UTDHandCard::OnCardPressedInternal);
		Btn->OnReleased.Clear();
		Btn->OnReleased.AddDynamic(this, &UTDHandCard::OnCardReleasedInternal);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
			FString::Printf(TEXT("[HandCard] Index=%d bound to Button"), CardIndex));
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
			FString::Printf(TEXT("[HandCard] Index=%d FAIL: no Button found"), CardIndex));
	}
}

void UTDHandCard::UpdateCost(float NewCost)
{
	CardCost = NewCost;
	if (CostText)
	{
		CostText->SetText(FText::AsNumber(int32(NewCost)));
	}
}

void UTDHandCard::OnCardPressedInternal()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
		FString::Printf(TEXT("[HandCard] OnCardPressedInternal Index=%d -> BeginPlacement"), CardIndex));

	ATDPlayerController* PC = Cast<ATDPlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->BeginPlacement(CardIndex);
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
			TEXT("[HandCard] FAIL: PC == null"));
	}
}

void UTDHandCard::OnCardReleasedInternal()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green,
		TEXT("[HandCard] OnCardReleasedInternal -> EndPlacement"));

	ATDPlayerController* PC = Cast<ATDPlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->EndPlacement();
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red,
			TEXT("[HandCard] FAIL: PC == null"));
	}
}
