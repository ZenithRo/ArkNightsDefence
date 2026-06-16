#include "UI/TDHealthBarWidget.h"
#include "Styling/SlateBrush.h"

static FProgressBarStyle CreateDefaultBarStyle()
{
	FProgressBarStyle Style;
	Style.BackgroundImage.TintColor = FLinearColor(0.05f, 0.05f, 0.05f, 0.7f);
	Style.BackgroundImage.DrawAs = ESlateBrushDrawType::Box;
	Style.FillImage.TintColor = FLinearColor::White;
	Style.FillImage.DrawAs = ESlateBrushDrawType::Box;
	return Style;
}

TSharedRef<SWidget> UTDHealthBarWidget::RebuildWidget()
{
	static const FProgressBarStyle BarStyle = CreateDefaultBarStyle();

	MyProgressBar = SNew(SProgressBar)
		.Style(&BarStyle)
		.Percent(TAttribute<TOptional<float>>::CreateLambda([this]() { return GetPercent(); }))
		.FillColorAndOpacity(DesiredColor);

	return MyProgressBar.ToSharedRef();
}

TOptional<float> UTDHealthBarWidget::GetPercent() const
{
	return CurrentPercent;
}

void UTDHealthBarWidget::SetHealthPercent(float Percent)
{
	CurrentPercent = FMath::Clamp(Percent, 0.0f, 1.0f);
}

void UTDHealthBarWidget::SetBarColor(const FLinearColor& Color)
{
	DesiredColor = Color;
	if (MyProgressBar.IsValid())
	{
		MyProgressBar->SetFillColorAndOpacity(Color);
	}
}
