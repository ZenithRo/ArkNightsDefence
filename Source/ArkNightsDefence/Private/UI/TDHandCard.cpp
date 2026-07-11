#include "UI/TDHandCard.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetTree.h"
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
	ATDPlayerController* PC = Cast<ATDPlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->BeginPlacement(CardIndex);
	}
}

void UTDHandCard::OnCardReleasedInternal()
{
	ATDPlayerController* PC = Cast<ATDPlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->EndPlacement();
	}
}
