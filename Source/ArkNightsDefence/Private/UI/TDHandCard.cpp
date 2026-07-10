#include "UI/TDHandCard.h"
#include "Components/Image.h"

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
}
