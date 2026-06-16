#include "TDHealthBarWidget.h"
#include "Styling/SlateBrush.h"

UTDHealthBarWidget::UTDHealthBarWidget()
{
	BarStyle.BackgroundImage.TintColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.7f);
	BarStyle.BackgroundImage.DrawAs = ESlateBrushDrawType::Box;
	BarStyle.FillImage.TintColor = FLinearColor::White;
	BarStyle.FillImage.DrawAs = ESlateBrushDrawType::Box;
}

TSharedRef<SWidget> UTDHealthBarWidget::RebuildWidget()
{
	MyProgressBar = SNew(SProgressBar)
		.Style(&BarStyle)
		.Percent(this, &UTDHealthBarWidget::GetPercent)
		.FillColorAndOpacity(FLinearColor::Red);

	return MyProgressBar.ToSharedRef();
}

float UTDHealthBarWidget::GetPercent() const
{
	return CurrentPercent;
}

void UTDHealthBarWidget::SetHealthPercent(float Percent)
{
	CurrentPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
}

void UTDHealthBarWidget::SetBarColor(const FLinearColor& Color)
{
	if (MyProgressBar.IsValid())
	{
		MyProgressBar->SetFillColorAndOpacity(Color);
	}
}
