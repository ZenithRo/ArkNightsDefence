#include "TDEnemy.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"

ATDEnemy::ATDEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetSphereRadius(50.0f);
	Collision->SetCollisionProfileName(TEXT("TDEnemy"));
	RootComponent = Collision;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Collision);
}

void ATDEnemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void ATDEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!TargetPath) return;

	DistanceAlongSpline += MoveSpeed * DeltaTime;

	float SplineLength = TargetPath->GetSplineLength();
	FVector NewLocation = TargetPath->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	FRotator NewRotation = TargetPath->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	SetActorLocationAndRotation(NewLocation, NewRotation);

	if (DistanceAlongSpline >= SplineLength)
	{
		OnReachedEnd();
	}
}

float ATDEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = DamageAmount - Armor;
	if (ActualDamage < 1.0f) ActualDamage = 1.0f;

	CurrentHealth -= ActualDamage;

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
}

void ATDEnemy::Die()
{
	Destroy();
}

void ATDEnemy::OnReachedEnd()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Enemy reached the end!"));
	}
	Destroy();
}