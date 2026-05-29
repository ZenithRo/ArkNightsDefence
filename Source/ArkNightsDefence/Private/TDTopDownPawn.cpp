#include "TDTopDownPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ATDTopDownPawn::ATDTopDownPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 2000.0f;
	SpringArm->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ATDTopDownPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ATDTopDownPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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

void ATDTopDownPawn::Move(const FInputActionValue& Value)
{
	FVector2D Input = Value.Get<FVector2D>();
	if (Input.IsNearlyZero()) return;

	FVector Forward = Camera->GetForwardVector();
	Forward.Z = 0.0f;
	Forward.Normalize();

	FVector Right = Camera->GetRightVector();
	Right.Z = 0.0f;
	Right.Normalize();

	FVector Delta = (Forward * Input.Y + Right * Input.X) * MoveSpeed * GetWorld()->GetDeltaSeconds();
	AddActorWorldOffset(Delta);
}

void ATDTopDownPawn::Zoom(const FInputActionValue& Value)
{
	float Wheel = Value.Get<float>();
	float NewLength = SpringArm->TargetArmLength - Wheel * ZoomStep;
	SpringArm->TargetArmLength = FMath::Clamp(NewLength, MinZoom, MaxZoom);
}
