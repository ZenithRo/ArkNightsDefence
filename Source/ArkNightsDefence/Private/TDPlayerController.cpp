// 玩家控制器实现: 鼠标显示, 输入映射注册, 点击射线检测
#include "TDPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

void ATDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 显示鼠标光标
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	if (!DefaultMappingContext) return;

	// 通过EnhancedInput子系统注册输入映射上下文
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATDPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 绑定鼠标点击事件 (Started: 按下瞬间触发, 防止一次点击多次触发)
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickAction)
		{
			EnhancedInput->BindAction(ClickAction, ETriggerEvent::Started, this, &ATDPlayerController::OnClick);
		}
	}
}

// 鼠标点击处理: 射线检测地面 → 打印坐标 → 画Debug红球
void ATDPlayerController::OnClick(const FInputActionValue& Value)
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (Hit.bBlockingHit)
	{
		FVector Location = Hit.Location;

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
				FString::Printf(TEXT("Clicked: X=%.0f Y=%.0f Z=%.0f"), Location.X, Location.Y, Location.Z));
		}

		// 在点击位置绘制红色调试球体 (后续替换为塔部署逻辑)
		DrawDebugSphere(GetWorld(), Location, 30.0f, 12, FColor::Red, false, 1.5f);
	}
}
