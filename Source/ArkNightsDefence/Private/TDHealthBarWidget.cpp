// 纯C++血条Widget: 程序化创建ProgressBar, 供WidgetComponent使用
#include "TDHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Blueprint/WidgetTree.h"

void UTDHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!HealthBar && WidgetTree)
	{
		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
		WidgetTree->RootWidget = HealthBar;

		HealthBar->SetPercent(Percent);
		HealthBar->SetFillColorAndOpacity(BarColor);
	}
}

void UTDHealthBarWidget::SetPercent(float NewPercent)
{
	Percent = FMath::Clamp(NewPercent, 0.0f, 1.0f);
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
	}
}

void UTDHealthBarWidget::SetBarColor(const FLinearColor& NewColor)
{
	BarColor = NewColor;
	if (HealthBar)
	{
		HealthBar->SetFillColorAndOpacity(BarColor);
	}
}
