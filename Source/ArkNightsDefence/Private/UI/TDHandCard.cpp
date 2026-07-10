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

	if (CardAvatarImage && CardAvatar)
	{
		CardAvatarImage->SetBrushFromTexture(CardAvatar);
		CardAvatarImage->SetDesiredSizeOverride(FVector2D(180.0f, 180.0f));
	}
}
