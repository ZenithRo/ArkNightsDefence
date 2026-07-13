#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tower/TDAttackRange.h"
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
enum class EEnemyType : uint8
{
	Land	UMETA(DisplayName = "地面"),
	Fly		UMETA(DisplayName = "飞行")
};

UENUM(BlueprintType)
enum class EEnemyAttackTarget : uint8
{
	Ground		UMETA(DisplayName = "仅地面塔"),
	Highland	UMETA(DisplayName = "仅高台塔"),
	Both		UMETA(DisplayName = "均可")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	EEnemyType EnemyType = EEnemyType::Land;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	EEnemyAttackTarget AttackTargetType = EEnemyAttackTarget::Both;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Defense")
	float PhysicalArmor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Defense")
	float MagicResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 ExperienceDrop = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 LifeDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float PhysicalDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float MagicDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float AttackInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float MeleeRange = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy|Attack")
	EAttackRangeMode AttackRangeMode = EAttackRangeMode::Circle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Enemy|Attack")
	TArray<FAttackRangeCell> AttackRangeCells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	TObjectPtr<AActor> PathActor;

	UPROPERTY(BlueprintReadOnly, Category = "Path")
	float DistanceAlongSpline = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<ATDBaseTower> CurrentTargetTower;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyDamage(float InPhysical, float InMagic);

	void ApplyDamageToSelf(float InPhysical, float InMagic);

	// 伤害计算(支持无视防御百分比: 0-100)
	void ApplyDamageToSelfWithPenetration(float InPhysical, float InMagic, float IgnorePhysPct, float IgnoreMagicPct);

	// 波次系统事件
	FSimpleMulticastDelegate OnEnemyFinished;
	FSimpleMulticastDelegate OnEnemyReachedEndDel;

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetPathActor(AActor* InPathActor);

	// 阻挡系统
	bool bIsBlocked = false;
	TWeakObjectPtr<ATDBaseTower> BlockedByTower;

	bool IsTowerInRangeCells(const ATDBaseTower* Tower) const;

	void OnBlocked(ATDBaseTower* Blocker);
	void OnUnblocked();

	// 减速系统
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplySlow(float SlowPercent, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void RemoveSlow();

	UPROPERTY(BlueprintReadOnly, Category = "Enemy")
	float CurrentMoveSpeedMultiplier = 1.0f;

private:
	TObjectPtr<USplineComponent> CachedSpline;
	FTimerHandle SlowTimerHandle;
	bool bIsDead = false;

	void PlayAnim(const FString& AnimName, bool Loop);
	FString ResolveMoveAnim(const FString& MoveType) const;

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
