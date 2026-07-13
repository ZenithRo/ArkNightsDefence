#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy/TDEnemy.h"
#include "Tower/TDAttackRange.h"
#include "Tower/TDDeployDirection.h"
#include "Grid/TDGridEnums.h"
#include "Tower/TDTowerLevelStats.h"
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

UENUM(BlueprintType)
enum class ETowerAttackMode : uint8
{
	SingleTarget	UMETA(DisplayName = "单体攻击"),
	AoE				UMETA(DisplayName = "群体攻击")
};

UENUM(BlueprintType)
enum class ETowerClass : uint8
{
	Vanguard    UMETA(DisplayName = "Vanguard"),
	Guard       UMETA(DisplayName = "Guard"),
	Caster      UMETA(DisplayName = "Caster"),
	Sniper      UMETA(DisplayName = "Sniper"),
	Defender    UMETA(DisplayName = "Defender"),
	Medic       UMETA(DisplayName = "Medic"),
	Supporter   UMETA(DisplayName = "Supporter"),
	Specialist  UMETA(DisplayName = "Specialist")
};

UCLASS(Blueprintable)
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

	// 后背视角Spine骨骼数据(朝上时使用, 由蓝图子类设置)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Spine")
	TObjectPtr<USpineSkeletonDataAsset> SkeletonDataAssetBack;

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
	float PhysicalDamage = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float MagicDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Defense")
	float PhysicalArmor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Defense")
	float MagicResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float AttackRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float AttackInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower")
	float CostToDeploy = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower")
	ETowerClass TowerClass = ETowerClass::Vanguard;

	UPROPERTY(BlueprintReadOnly, Category = "Tower|Upgrade")
	int32 TowerLevel = 1;

	// 每一级的完整属性(Index 0 = Lv1默认, 1 = Lv2, 2 = Lv3)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower|Upgrade")
	TArray<FTowerLevelStats> LevelStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	int32 UpgradeCost_Lv2 = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Upgrade")
	int32 UpgradeCost_Lv3 = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower|Attack")
	TArray<FAttackRangeCell> AttackRangeCells;

	// 最大阻挡数(敌人被该塔阻挡的数量上限)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat")
	int32 MaxBlockCount = 1;

	// 目标选择器(优先级策略)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower|Combat")
	TObjectPtr<class UTDGTargetSelector> TargetSelector;

	// 部署方向
	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	EDeployDirection DeployDirection = EDeployDirection::LEFT;

	// 部署类型(地面/高台/均可)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower")
	ETowerPlacement PlacementType = ETowerPlacement::GROUND_ONLY;

	// 攻击目标类型(地面/飞行/均可)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat")
	EAttackTargetType AttackTargetType = EAttackTargetType::Land;

	// 是否为医疗塔(治疗友方塔)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat")
	bool bIsMedic = false;

	// 医疗塔: 治疗量 = 额外治疗量 + PhysicalDamage + MagicDamage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat", meta = (EditCondition = "bIsMedic"))
	float HealAmount = 30.0f;

	// 攻击模式: 单体(默认) / 群体
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower|Combat")
	ETowerAttackMode AttackMode = ETowerAttackMode::SingleTarget;

	// 减速百分比(0=不减速, 50=减速50%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat")
	float SlowPercentage = 0.0f;

	// 减速持续秒数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat")
	float SlowDuration = 2.0f;

	// 无视物理防御百分比(0=不无视, 50=无视50%, 100=无视全部)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat")
	float IgnorePhysicalArmorPercent = 0.0f;

	// 无视法术防御百分比
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat")
	float IgnoreMagicResistancePercent = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Tower|Attack")
	EAttackRangeMode AttackRangeMode = EAttackRangeMode::Circle;

	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	TObjectPtr<ATDEnemy> CurrentTarget;

	// 医疗塔当前治疗目标
	UPROPERTY(BlueprintReadOnly, Category = "Tower")
	TObjectPtr<ATDBaseTower> HealTarget;

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void FindTarget();

	UFUNCTION(BlueprintCallable, Category = "Tower")
	bool LevelUp();

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void SetLevelDirectly(int32 NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void Fire();

	void Die();

	void SetGridCoordinate(int32 Col, int32 Row);

	void SetDeployDirection(EDeployDirection NewDir);

	int32 GridCol = -1;
	int32 GridRow = -1;

	int32 GetCurrentBlockCount() const;

	TArray<TWeakObjectPtr<ATDEnemy>> BlockedEnemies;

	void AddBlockedEnemy(ATDEnemy* Enemy);

	void RemoveBlockedEnemy(ATDEnemy* Enemy);

	void FreeAllBlockedEnemies();

	bool CanTargetEnemy(const ATDEnemy* Enemy) const;

protected:
	UFUNCTION()
	void OnAnimComplete(UTrackEntry* Entry);

	void PlayAnim(const FString& AnimName, bool Loop);

private:
	FTimerHandle FireTimerHandle;
	int32 GetUpgradeCost() const;
	void ApplyLevelUpStats();

	bool IsEnemyInRangeCells(ATDEnemy* Enemy) const;
	bool IsTowerInRangeCells(ATDBaseTower* Ally) const;
};
