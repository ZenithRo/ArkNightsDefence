#include "TDHealthBarWidget.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"

TSharedRef<SWidget> UTDHealthBarWidget::RebuildWidget()
{
	TSharedRef<SBorder> Background = SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.7f))
		.Padding(FMargin(1.0f));

	MyProgressBar = SNew(SProgressBar)
		.Percent(1.0f)
		.FillColorAndOpacity(FLinearColor::Red)
		.BackgroundImage(nullptr);

	Background->SetContent(MyProgressBar.ToSharedRef());

	return Background;
}

void UTDHealthBarWidget::SetHealthPercent(float Percent)
{
	if (MyProgressBar.IsValid())
	{
		MyProgressBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

void UTDHealthBarWidget::SetBarColor(const FLinearColor& Color)
{
	if (MyProgressBar.IsValid())
	{
		MyProgressBar->SetFillColorAndOpacity(Color);
	}
}
