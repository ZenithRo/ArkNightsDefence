// UMG HUD Widget实现: 由GameMode数据变化时触发更新, 不依赖Tick轮询
#include "TDHUDWidget.h"
#include "TDGameMode.h"
#include "Components/TextBlock.h"

// 从GameMode读取最新数据并刷新所有TextBlock (被GameMode在数据变化时调用)
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
}
