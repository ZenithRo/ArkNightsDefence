#include "TDHealthBarWidget.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"

void UTDHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!HealthBar && WidgetTree)
	{
		UCanvasPanel* RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = RootPanel;

		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));

		UCanvasPanelSlot* Slot = RootPanel->AddChildToCanvas(HealthBar);
		if (Slot)
		{
			Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			Slot->SetOffsets(FMargin(0.0f));
			Slot->SetAlignment(FVector2D(0.0f, 0.0f));
		}

		// Style: dark background + colored fill
		FProgressBarStyle Style;

		FSlateBrush BackgroundBrush;
		BackgroundBrush.TintColor = FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.7f));
		BackgroundBrush.DrawAs = ESlateBrushDrawType::Box;
		Style.SetBackgroundImage(BackgroundBrush);

		FSlateBrush FillBrush;
		FillBrush.TintColor = FSlateColor(FLinearColor::Green);
		FillBrush.DrawAs = ESlateBrushDrawType::Box;
		Style.SetFillImage(FillBrush);

		HealthBar->WidgetStyle = Style;
		HealthBar->SetPercent(1.0f);

		// 默认填充方向从左到右(水平方向)
		HealthBar->BarFillType = EProgressBarFillType::LeftToRight;
	}
}

void UTDHealthBarWidget::SetHealthPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}
}

void UTDHealthBarWidget::SetBarColor(const FLinearColor& Color)
{
	if (HealthBar)
	{
		FProgressBarStyle Style = HealthBar->WidgetStyle;
		FSlateBrush FillBrush = Style.GetFillImage();
		FillBrush.TintColor = FSlateColor(Color);
		Style.SetFillImage(FillBrush);
		HealthBar->WidgetStyle = Style;
	}
}
