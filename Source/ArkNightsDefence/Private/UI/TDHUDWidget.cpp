#include "UI/TDHUDWidget.h"
#include "Core/TDGameMode.h"
#include "Core/TDPlayerController.h"
#include "Components/TextBlock.h"

void UTDHUDWidget::UpdateDisplay()
{
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	if (TextLives)
	{
		TextLives->SetText(FText::FromString(FString::Printf(TEXT("Lives: %d"), GM->PlayerLives)));
	}
	if (TextCost)
	{
		TextCost->SetText(FText::FromString(FString::Printf(TEXT("Cost: %.0f"), GM->Cost)));
	}
	if (TextExp)
	{
		TextExp->SetText(FText::FromString(FString::Printf(TEXT("EXP: %d"), GM->Experience)));
	}
	if (TextWave)
	{
		FString WaveText = FString::Printf(TEXT("Wave %d: %d/%d"),
			GM->CurrentWaveIndex + 1, GM->WaveKilledCount, GM->WaveTotalCount);
		TextWave->SetText(FText::FromString(WaveText));
	}
}

void UTDHUDWidget::OnPauseButtonClicked()
{
	ATDPlayerController* PC = Cast<ATDPlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->OnPauseToggle();
	}
}

void UTDHUDWidget::OnUpgradeButtonClicked()
{
	ATDPlayerController* PC = Cast<ATDPlayerController>(GetOwningPlayer());
	if (!PC) return;

	if (PC->IsInUpgradeMode())
	{
		PC->ExitUpgradeMode();
	}
	else
	{
		PC->EnterUpgradeMode();
	}
}
