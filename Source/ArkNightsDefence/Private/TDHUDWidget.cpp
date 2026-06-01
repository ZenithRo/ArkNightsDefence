// UMG HUD Widget实现: 实时显示生命/费用/经验
#include "TDHUDWidget.h"
#include "TDGameMode.h"
#include "Components/TextBlock.h"

// 每帧从GameMode读取数据并更新TextBlock
void UTDHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
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
}
