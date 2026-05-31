#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDEnemy.generated.h"

class USplineComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class ARKNIGHTSDEFENCE_API ATDEnemy : public AActor
{
	GENERATED_BODY()

public:
	ATDEnemy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float Armor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 BountyGold = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	TObjectPtr<USplineComponent> TargetPath;

	UPROPERTY(BlueprintReadOnly, Category = "Path")
	float DistanceAlongSpline = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UFUNCTION()
	void Die();

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void OnReachedEnd();
};