#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDEnemy.generated.h"

class USplineComponent;
class USphereComponent;
class UStaticMeshComponent;
class USpineSkeletonAnimationComponent;
class USpineSkeletonDataAsset;
class ATDBaseTower;
class UTrackEntry;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical	UMETA(DisplayName = "物理伤害"),
	Magic		UMETA(DisplayName = "法术伤害")
};

UENUM(BlueprintType)
enum class EEnemyAnimState : uint8
{
	None,
	Idle,
	MoveBeginning,
	Moving,
	MoveEnding,
	Attacking,
	Dying
};

USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ATDBaseTower* SourceTower = nullptr;
};

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
	virtual void Destroyed() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpineSkeletonAnimationComponent> SpineAnim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UWidgetComponent> HealthBarComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Spine")
	TObjectPtr<USpineSkeletonDataAsset> SkeletonDataAsset;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy|Spine")
	EEnemyAnimState AnimState = EEnemyAnimState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float MoveSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Defense")
	float PhysicalArmor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Defense")
	float MagicResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 ExperienceDrop = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 LifeDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float AttackInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float MeleeRange = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	TObjectPtr<AActor> PathActor;

	UPROPERTY(BlueprintReadOnly, Category = "Path")
	float DistanceAlongSpline = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<ATDBaseTower> CurrentTargetTower;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyDamage(float DamageAmount, EDamageType DamageType);

	// 阻挡系统
	bool bIsBlocked = false;
	TWeakObjectPtr<ATDBaseTower> BlockedByTower;

	void OnBlocked(ATDBaseTower* Blocker);
	void OnUnblocked();

private:
	TObjectPtr<USplineComponent> CachedSpline;
	FTimerHandle AttackTimerHandle;
	bool bIsDead = false;

	void PlayAnim(const FString& AnimName, bool Loop);

	UFUNCTION()
	void OnAnimComplete(UTrackEntry* Entry);

	UFUNCTION()
	void MeleeAttack();

	void FindNearestTower();

	void Die();

protected:
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void OnReachedEnd();
};
