// 纯C++血条Widget: CanvasPanel根+ProgressBar子, 无需UMG蓝图
#include "TDHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"

void UTDHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!HealthBar && WidgetTree)
	{
		// 创建CanvasPanel作为根, 保证子Widget有正确的布局
		UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;

		// 创建ProgressBar并添加到CanvasPanel, 锚定四角充满全屏
		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
		UCanvasPanelSlot* Slot = Root->AddChildToCanvas(HealthBar);
		Slot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		Slot->SetOffsets(FMargin(0.0f));
		Slot->SetAlignment(FVector2D(0.0f, 0.0f));

		HealthBar->SetPercent(Percent);
		HealthBar->SetFillColorAndOpacity(BarColor);
	}
}

void UTDHealthBarWidget::UpdateBar()
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
		HealthBar->SetFillColorAndOpacity(BarColor);
	}
}

void UTDHealthBarWidget::SetPercent(float NewPercent)
{
	Percent = FMath::Clamp(NewPercent, 0.0f, 1.0f);
	UpdateBar();
}

void UTDHealthBarWidget::SetBarColor(const FLinearColor& NewColor)
{
	BarColor = NewColor;
	UpdateBar();
}
