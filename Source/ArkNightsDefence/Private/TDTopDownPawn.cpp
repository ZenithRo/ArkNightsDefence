// 俯视角摄像机Pawn实现: WASD地图平移 + 滚轮缩放
#include "TDTopDownPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ATDTopDownPawn::ATDTopDownPawn()
{
	// 不需要Tick, 移动由输入事件驱动
	PrimaryActorTick.bCanEverTick = false;

	// 创建弹簧臂, 设置默认臂长和碰撞行为
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 2000.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// 创建摄像机, 挂载到弹簧臂末端
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ATDTopDownPawn::BeginPlay()
{
	Super::BeginPlay();
	// 设置摄像机俯角 (从蓝图ClassDefaults读取CameraPitch)
	SpringArm->SetRelativeRotation(FRotator(CameraPitch, 0.0f, 0.0f));
}

void ATDTopDownPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 使用EnhancedInput绑定移动和缩放
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATDTopDownPawn::Move);
		}
		if (ZoomAction)
		{
			EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATDTopDownPawn::Zoom);
		}
	}
}

// WASD水平平移: 基于摄像机朝向投影到水平面, 帧率无关
void ATDTopDownPawn::Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (Input.IsNearlyZero()) return;

	// 获取摄像机前方向量, Z分量清零以保持水平移动
	FVector Forward = Camera->GetForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();

	FVector Right = Camera->GetRightVector();
	Right.Z = 0.0f;
	Right.Normalize();

	// Y=前后(W/S), X=左右(A/D), 乘以速度和时间
	FVector Delta = (Forward * Input.Y + Right * Input.X) * MoveSpeed * GetWorld()->GetDeltaSeconds();
	AddActorWorldOffset(Delta);
}

// 滚轮缩放: 前滚=拉近, 后滚=拉远
void ATDTopDownPawn::Zoom(const FInputActionValue& Value)
{
	float Wheel = Value.Get<float>();
	if (Wheel > 0.0f)
	{
		SpringArm->TargetArmLength = CloseZoom;
	}
	else if (Wheel < 0.0f)
	{
		SpringArm->TargetArmLength = FarZoom;
	}
}
