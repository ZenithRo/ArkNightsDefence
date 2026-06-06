// 玩家控制器实现: 鼠标显示, 输入映射注册, 点击部署塔
#include "TDPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "TDGameMode.h"
#include "TDBaseTower.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

void ATDPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	if (!DefaultMappingContext) return;

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

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickAction)
		{
			EnhancedInput->BindAction(ClickAction, ETriggerEvent::Started, this, &ATDPlayerController::OnClick);
		}
	}
}

// 鼠标点击: 射线检测地面 → SpendCost → SpawnActor生成塔
void ATDPlayerController::OnClick(const FInputActionValue& Value)
{
	if (!TowerToDeploy) return;

	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (!Hit.bBlockingHit) return;

	// 尝试扣费
	ATDGameMode* GM = Cast<ATDGameMode>(GetWorld()->GetAuthGameMode());
	if (!GM) return;

	if (!GM->SpendCost(TowerToDeploy.GetDefaultObject()->CostToDeploy)) return;

	// 扣费成功, 在点击位置生成塔, 直接使用Hit.Location吸附到地面碰撞
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATDBaseTower* Tower = GetWorld()->SpawnActor<ATDBaseTower>(TowerToDeploy, Hit.Location, FRotator::ZeroRotator, SpawnParams);

	if (Tower)
	{
		DrawDebugSphere(GetWorld(), Hit.Location, 30.0f, 12, FColor::Green, false, 1.5f);
	}
}
