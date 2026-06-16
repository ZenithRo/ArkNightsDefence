#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy/TDEnemy.h"
#include "Tower/TDAttackRange.h"
#include "TDBaseTower.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class ATDEnemy;
class USpineSkeletonAnimationComponent;
class USpineSkeletonDataAsset;
class UTrackEntry;
class UWidgetComponent;
class UTDHealthBarWidget;

UENUM(BlueprintType)
enum class ETowerAnimState : uint8
{
	None,
	Starting,
	Idle,
	AttackStarting,
	Attacking,
	AttackEnding,
	Dying
};

UCLASS()
class ARKNIGHTSDEFENCE_API ATDBaseTower : public AActor
{
	GENERATED_BODY()

public:
	ATDBaseTower();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> RangeSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> MeleeRangeSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpineSkeletonAnimationComponent> SpineAnim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> HealthBarComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Spine")
	TObjectPtr<USpineSkeletonDataAsset> SkeletonDataAsset;

	UPROPERTY(BlueprintReadOnly, Category = "Tower|Spine")
	ETowerAnimState AnimState = ETowerAnimState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float AttackDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float AttackRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float AttackInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	EDamageType DamageType = EDamageType::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float CostToDeploy = 10.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Tower|Upgrade")
	int32 TowerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	int32 UpgradeCost_Lv2 = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	int32 UpgradeCost_Lv3 = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	float DamagePerLevel = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	float IntervalReducePerLevel = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	float RangePerLevel = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower|Attack")
	TArray<FAttackRangeCell> AttackRangeCells;

	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	TObjectPtr<ATDEnemy> CurrentTarget;

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void FindTarget();

	UFUNCTION(BlueprintCallable, Category = "Tower")
	bool LevelUp();

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void Fire();

	void Die();

	void SetGridCoordinate(int32 Col, int32 Row);

	int32 GridCol = -1;
	int32 GridRow = -1;

protected:
	UFUNCTION()
	void OnAnimComplete(UTrackEntry* Entry);

	void PlayAnim(const FString& AnimName, bool Loop);

private:
	FTimerHandle FireTimerHandle;
	int32 GetUpgradeCost() const;
	void ApplyLevelUpStats();

	bool IsEnemyInRangeCells(ATDEnemy* Enemy) const;
};
