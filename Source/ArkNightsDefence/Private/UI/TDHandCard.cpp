#include "UI/TDHandCard.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetTree.h"
#include "Core/TDPlayerController.h"
#include "Animation/WidgetAnimation.h"

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

	// 定时检查升级模式切换(0.15秒间隔), 控制 Update 动画的播放/停止
	GetWorld()->GetTimerManager().SetTimer(UpgradeCheckTimer, this, &UTDHandCard::UpdateUpgradeAnim, 0.15f, true);
}

void UTDHandCard::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(UpgradeCheckTimer);
	Super::NativeDestruct();
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
	if (!PC) return;

	if (PC->IsInUpgradeMode())
	{
		int32 Result = PC->TryUpgradeHandCard(CardIndex);
		PlayResultAnim(Result);
		if (Result == 0)
		{
			PC->ExitUpgradeMode();
		}
	}
	else
	{
		PC->BeginPlacement(CardIndex);
	}
}

void UTDHandCard::OnCardReleasedInternal()
{
	ATDPlayerController* PC = Cast<ATDPlayerController>(GetOwningPlayer());
	if (!PC || PC->IsInUpgradeMode()) return;

	PC->EndPlacement();
}

void UTDHandCard::PlayResultAnim(int32 Result)
{
	if (Result == 0 && AnimSuccess)
	{
		if (AnimUpdate && IsAnimationPlaying(AnimUpdate))
		{
			StopAnimation(AnimUpdate);
		}
		bWasInUpgradeMode = false;
		PlayAnimation(AnimSuccess);
	}
	else if (Result != 0 && AnimFalse)
	{
		PlayAnimation(AnimFalse);
	}
}

void UTDHandCard::UpdateUpgradeAnim()
{
	ATDPlayerController* PC = Cast<ATDPlayerController>(GetOwningPlayer());
	if (!PC) return;

	bool bIsUpgrade = PC->IsInUpgradeMode();

	if (bIsUpgrade && !bWasInUpgradeMode)
	{
		if (AnimUpdate)
		{
			PlayAnimation(AnimUpdate, 0.0f, 0);
		}
	}
	else if (!bIsUpgrade && bWasInUpgradeMode)
	{
		if (AnimUpdate && IsAnimationPlaying(AnimUpdate))
		{
			StopAnimation(AnimUpdate);
		}
	}

	bWasInUpgradeMode = bIsUpgrade;
}
