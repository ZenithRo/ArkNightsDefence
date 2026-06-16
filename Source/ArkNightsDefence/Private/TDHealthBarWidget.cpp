#include "TDHealthBarWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateColor.h"

void UTDHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (WidgetTree)
	{
		UCanvasPanel* RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = RootPanel;

		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
		UCanvasPanelSlot* BarSlot = RootPanel->AddChildToCanvas(HealthBar);
		if (BarSlot)
		{
			BarSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			BarSlot->SetOffsets(FMargin(0.0f));
			BarSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		}

		FProgressBarStyle Style;
		FSlateBrush BackgroundBrush;
		BackgroundBrush.TintColor = FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.7f));
		BackgroundBrush.DrawAs = ESlateBrushDrawType::Box;
		Style.BackgroundImage = BackgroundBrush;

		FSlateBrush FillBrush;
		FillBrush.TintColor = FSlateColor(FLinearColor::Green);
		FillBrush.DrawAs = ESlateBrushDrawType::Box;
		Style.FillImage = FillBrush;

		HealthBar->SetWidgetStyle(Style);
		HealthBar->SetPercent(1.0f);
		HealthBar->SetFillDirection(EProgressBarFillType::LeftToRight);
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
		FProgressBarStyle Style = HealthBar->GetWidgetStyle();
		Style.FillImage.TintColor = FSlateColor(Color);
		HealthBar->SetWidgetStyle(Style);
	}
}
