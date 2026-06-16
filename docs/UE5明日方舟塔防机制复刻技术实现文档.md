# UE5明日方舟塔防机制复刻技术实现文档

**版本**: 1\.0
**引擎版本**: Unreal Engine 5\.7\+
**适用范围**: 关卡内战斗系统实现

---

## 目录

1. \[关卡格子系统\]\(\#1\-关卡格子系统\)

2. \[部署系统\]\(\#2\-部署系统\)

3. \[敌人路径与 AI 系统\]\(\#3\-敌人路径与ai系统\)

4. \[攻击系统\]\(\#4\-攻击系统\)

5. \[技能系统\]\(\#5\-技能系统\)

6. \[目标选择优先级系统\]\(\#6\-目标选择优先级系统\)

---

## 1\. 关卡格子系统

### 1\.1 设计思路

明日方舟采用**六边形网格**变种的**等距菱形格子**系统，核心特点：

- 双层地形：地面格（近战）\+ 高台格（远程）

- 地形类型掩码系统（tileTypesMask）

- 格子坐标采用轴向坐标系（Axial Coordinates）

- 每个格子独立存储部署状态、阻挡状态、地形效果

### 1\.2 数据结构定义

#### C\+\+ 数据结构

```cpp
// GridSystem/GridCell.h
UENUM(BlueprintType)
enum class ETileType : uint8
{
    NONE                = 0,
    GROUND              = 1 << 0,   // 地面可部署
    HIGHLAND            = 1 << 1,   // 高台可部署
    START               = 1 << 2,   // 敌人出生点
    END                 = 1 << 3,   // 终点
    HOLE                = 1 << 4,   // 坑洞
    BLOCKED             = 1 << 5,   // 不可通行
    FAKE_HIGHLAND       = 1 << 6,   // 假高台
    GRASS               = 1 << 7,   // 草丛（隐匿）
    MEDICAL_RUNE        = 1 << 8,   // 医疗符文
    AIR_DEFENSE_RUNE    = 1 << 9,   // 防空符文
    DEFENSE_RUNE        = 1 << 10   // 防御符文
};

USTRUCT(BlueprintType)
struct FGridCell
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FIntPoint Coordinate;           // 格子坐标 (Q, R)
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ETileType TileType;             // 地形类型掩码
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsOccupied;               // 是否已被部署
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TWeakObjectPtr<class AOperatorBase> OccupiedOperator;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<TWeakObjectPtr<class AEnemyBase>> BlockingEnemies;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 MaxBlockCount;            // 最大阻挡数
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector WorldPosition;          // 世界空间位置
};
```

#### 格子管理器单例

```cpp
// GridSystem/GridManager.h
UCLASS()
class AGridManager : public AActor
{
    GENERATED_BODY()
    
public:
    // 格子尺寸（明日方舟标准：128单位）
    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    float CellSize = 128.0f;
    
    // 网格尺寸
    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    int32 GridWidth = 9;
    
    UPROPERTY(EditDefaultsOnly, Category = "Grid")
    int32 GridHeight = 8;
    
    // 获取格子
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FGridCell* GetCellByCoordinate(FIntPoint Coordinate);
    
    // 世界坐标转格子坐标
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FIntPoint WorldToGrid(FVector WorldPosition);
    
    // 格子坐标转世界坐标
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FVector GridToWorld(FIntPoint Coordinate);
    
    // 检查是否可部署
    UFUNCTION(BlueprintCallable, Category = "Grid")
    bool CanDeployAt(FIntPoint Coordinate, EOperatorClass OperatorClass);
    
private:
    TMap<FIntPoint, FGridCell> GridCells;
    
    void InitializeGrid();
};
```

### 1\.3 核心代码实现

#### 坐标转换算法

```cpp
FIntPoint AGridManager::WorldToGrid(FVector WorldPosition)
{
    // 等距菱形格子转换算法
    float q = (WorldPosition.X * FMath::Sqrt(3) / 3 - WorldPosition.Y / 3) / CellSize;
    float r = (WorldPosition.Y * 2 / 3) / CellSize;
    
    // 轴向坐标取整
    return HexRound(q, r);
}

FIntPoint AGridManager::HexRound(float q, float r)
{
    float s = -q - r;
    int32 rq = FMath::RoundToInt(q);
    int32 rr = FMath::RoundToInt(r);
    int32 rs = FMath::RoundToInt(s);
    
    float qDiff = FMath::Abs(rq - q);
    float rDiff = FMath::Abs(rr - r);
    float sDiff = FMath::Abs(rs - s);
    
    if (qDiff > rDiff && qDiff > sDiff)
        rq = -rr - rs;
    else if (rDiff > sDiff)
        rr = -rq - rs;
    
    return FIntPoint(rq, rr);
}
```

#### 可部署性检查

```cpp
bool AGridManager::CanDeployAt(FIntPoint Coordinate, EOperatorClass OperatorClass)
{
    FGridCell* Cell = GetCellByCoordinate(Coordinate);
    if (!Cell || Cell->bIsOccupied) return false;
    
    // 检查职业与地形匹配
    bool bIsGround = (Cell->TileType & ETileType::GROUND) != ETileType::NONE;
    bool bIsHighland = (Cell->TileType & ETileType::HIGHLAND) != ETileType::NONE;
    
    switch (OperatorClass)
    {
        case EOperatorClass::VANGUARD:
        case EOperatorClass::GUARD:
        case EOperatorClass::DEFENDER:
            return bIsGround;
            
        case EOperatorClass::SNIPER:
        case EOperatorClass::CASTER:
        case EOperatorClass::MEDIC:
        case EOperatorClass::SUPPORTER:
            return bIsHighland;
            
        case EOperatorClass::SPECIALIST:
            return bIsGround || bIsHighland;
            
        default:
            return false;
    }
}
```

### 1\.4 蓝图实现要点

1. **格子可视化组件**

    - 创建 `BP_GridCell` 作为格子可视化 Actor

    - 使用 `InstancedStaticMeshComponent` 批量渲染格子

    - 根据地形类型设置不同材质（地面：绿色，高台：黄色，不可部署：红色）

2. **运行时格子高亮**

    ```Plain Text
    Event OnMouseEnter → Set Material (HighlightMaterial)
    Event OnMouseLeave → Set Material (NormalMaterial)
    ```

3. **格子状态更新**

    - 部署时：`bIsOccupied = true`，更新材质为半透明灰色

    - 撤退时：`bIsOccupied = false`，恢复原始材质

### 1\.5 注意事项

- **坐标系统一致性**：所有系统必须使用统一的轴向坐标系，避免坐标转换误差

- **性能优化**：使用 `InstancedStaticMesh` 而非单独 Actor 渲染格子

- **地形掩码**：使用位运算支持多地形叠加（如同时具有草丛和地面属性）

- **内存管理**：格子数据使用 TMap 存储，避免稀疏数组浪费内存

---

## 2\. 部署系统

### 2\.1 设计思路

明日方舟部署系统核心机制：

- 干员手牌系统（待部署区）

- 拖拽式放置 \+ 方向选择

- 费用系统（自然回复 \+ 击杀回复 \+ 技能回复）

- 再部署冷却机制

- 部署方向影响攻击范围朝向

### 2\.2 数据结构定义

#### 干员数据结构

```cpp
// Operator/OperatorData.h
UENUM(BlueprintType)
enum class EOperatorClass : uint8
{
    VANGUARD,       // 先锋
    GUARD,          // 近卫
    DEFENDER,       // 重装
    SNIPER,         // 狙击
    CASTER,         // 术师
    MEDIC,          // 医疗
    SUPPORTER,      // 辅助
    SPECIALIST      // 特种
};

USTRUCT(BlueprintType)
struct FOperatorData
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString OperatorName;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    EOperatorClass OperatorClass;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 BaseCost;                 // 基础部署费用
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 RedeploymentTime;         // 再部署时间（秒）
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxBlockCount;            // 最大阻挡数
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float AttackInterval;           // 攻击间隔
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FIntPoint> AttackRange;  // 相对攻击范围坐标
    
    // 属性
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float MaxHP;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ATK;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float DEF;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float RES;                      // 法术抗性
};
```

#### 部署管理器

```cpp
// Deployment/DeploymentManager.h
UCLASS()
class ADeploymentManager : public AActor
{
    GENERATED_BODY()
    
public:
    // 费用系统
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentCost;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 MaxCost = 99;
    
    UPROPERTY(EditDefaultsOnly, Category = "Cost")
    float CostRegenInterval = 1.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Cost")
    int32 CostRegenAmount = 1;
    
    // 手牌系统
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<UOperatorDataAsset*> HandCards;
    
    // 部署预览
    UPROPERTY()
    class AOperatorPreview* PreviewOperator;
    
    // 事件分发
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOperatorDeployed, AOperatorBase*, DeployedOperator);
    UPROPERTY(BlueprintAssignable)
    FOnOperatorDeployed OnOperatorDeployed;
    
    UFUNCTION(BlueprintCallable)
    void StartDragDeployment(UOperatorDataAsset* OperatorData);
    
    UFUNCTION(BlueprintCallable)
    void ConfirmDeployment(FIntPoint GridCoord, EDirection Direction);
    
    UFUNCTION(BlueprintCallable)
    void CancelDeployment();
    
    UFUNCTION(BlueprintCallable)
    void RetreatOperator(AOperatorBase* Operator);
    
private:
    FTimerHandle CostRegenTimer;
    
    void StartCostRegen();
    void RegenCost();
};
```

### 2\.3 核心代码实现

#### Enhanced Input 拖拽系统

```cpp
// Input/TDInputConfig.h
// Input Action 定义
UENUM(BlueprintType)
enum class ETDInputID : uint32
{
    MouseClick = 0,
    MouseDrag,
    MouseRelease,
    RotatePreview
};

// 输入绑定实现
void UTDInputComponent::BindInputActions()
{
    // 鼠标按下 - 开始拖拽
    BindAction(IA_MouseClick, ETriggerEvent::Started, this,
        &UTDInputComponent::OnMouseClickStarted);
    
    // 拖拽中 - 更新预览位置
    BindAction(IA_MouseDrag, ETriggerEvent::Triggered, this,
        &UTDInputComponent::OnMouseDragging);
    
    // 鼠标释放 - 确认部署
    BindAction(IA_MouseClick, ETriggerEvent::Completed, this,
        &UTDInputComponent::OnMouseClickReleased);
    
    // R键 - 旋转部署方向
    BindAction(IA_RotatePreview, ETriggerEvent::Started, this,
        &UTDInputComponent::RotateDeployDirection);
}
```

#### 费用系统实现

```cpp
void ADeploymentManager::StartCostRegen()
{
    GetWorld()->GetTimerManager().SetTimer(
        CostRegenTimer,
        this,
        &ADeploymentManager::RegenCost,
        CostRegenInterval,
        true  // 循环执行
    );
}

void ADeploymentManager::RegenCost()
{
    if (CurrentCost < MaxCost)
    {
        CurrentCost = FMath::Min(CurrentCost + CostRegenAmount, MaxCost);
        OnCostChanged.Broadcast(CurrentCost);
    }
}

// 击杀敌人获得费用
void ADeploymentManager::AddCostOnKill(int32 KillCost)
{
    CurrentCost = FMath::Min(CurrentCost + KillCost, MaxCost);
}
```

#### 部署验证与执行

```cpp
bool ADeploymentManager::ConfirmDeployment(FIntPoint GridCoord, EDirection Direction)
{
    // 1. 费用检查
    if (CurrentCost < SelectedOperator->OperatorData.BaseCost)
    {
        CancelDeployment();
        return false;
    }
    
    // 2. 地形检查
    if (!GridManager->CanDeployAt(GridCoord, SelectedOperator->OperatorData.OperatorClass))
    {
        return false;
    }
    
    // 3. 扣除费用
    CurrentCost -= SelectedOperator->OperatorData.BaseCost;
    
    // 4. 生成干员Actor
    FActorSpawnParameters SpawnParams;
    AOperatorBase* NewOperator = GetWorld()->SpawnActor<AOperatorBase>(
        SelectedOperator->OperatorClass,
        GridManager->GridToWorld(GridCoord),
        DirectionToRotation(Direction),
        SpawnParams
    );
    
    // 5. 初始化干员
    NewOperator->InitializeOperator(SelectedOperator->OperatorData);
    NewOperator->SetGridCoordinate(GridCoord);
    NewOperator->SetDeployDirection(Direction);
    
    // 6. 更新格子状态
    GridManager->OccupyCell(GridCoord, NewOperator);
    
    // 7. 从手牌移除
    RemoveFromHand(SelectedOperator);
    
    OnOperatorDeployed.Broadcast(NewOperator);
    return true;
}
```

### 2\.4 蓝图实现要点

1. **干员手牌 UI**

    - 创建 `WBP_OperatorCard` 作为手牌控件

    - 重写 `OnMouseButtonDown` 启动拖拽操作

    - 使用 `UDragDropOperation` 传递干员数据

2. **部署预览 Actor**

    ```Plain Text
    Event Tick:
      → Get Mouse Position → Deproject to World
      → WorldToGrid() 获取格子坐标
      → 更新预览Actor位置到格子中心
      → 检查CanDeployAt()，设置预览材质（绿色=可部署，红色=不可部署）
    ```

3. **方向选择系统**

    - 按 R 键循环切换 4 个方向（上 / 右 / 下 / 左）

    - 实时旋转攻击范围预览 Mesh

    - 方向影响 AttackRange 坐标旋转变换

### 2\.5 注意事项

- **输入优先级**：拖拽输入需高于普通点击，使用 `InputPriority` 机制

- **预览性能**：预览 Actor 使用半透明材质，禁用碰撞和 tick 开销

- **费用同步**：多人游戏中费用变化需通过 RPC 同步

- **再部署冷却**：使用 `GameplayEffect` 实现冷却计时，支持 GAS 集成

---

## 3\. 敌人路径与 AI 系统

### 3\.1 设计思路

明日方舟敌人 AI 核心特点：

- Spline Component 实现平滑路径跟随

- SPFA（Shortest Path Faster Algorithm）动态寻路

- 波次管理器控制敌人生成时序

- 阻挡机制：被干员阻挡时停止移动

- 状态机：移动 → 被阻挡 → 攻击 → 死亡

### 3\.2 数据结构定义

#### 路径点数据

```cpp
// Enemy/PathSpline.h
UCLASS()
class APathSpline : public AActor
{
    GENERATED_BODY()
    
public:
    APathSpline();
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class USplineComponent* PathSpline;
    
    // 获取路径总长度
    UFUNCTION(BlueprintCallable, Category = "Path")
    float GetPathLength() const { return PathSpline->GetSplineLength(); }
    
    // 获取指定距离处的位置
    UFUNCTION(BlueprintCallable, Category = "Path")
    FVector GetLocationAtDistance(float Distance) const;
    
    // 获取指定距离处的旋转
    UFUNCTION(BlueprintCallable, Category = "Path")
    FRotator GetRotationAtDistance(float Distance) const;
};
```

#### 敌人基类

```cpp
// Enemy/EnemyBase.h
UCLASS()
class AEnemyBase : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()
    
public:
    AEnemyBase();
    
    // GAS 集成
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UAbilitySystemComponent* AbilitySystemComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UEnemyAttributeSet* AttributeSet;
    
    // 路径跟随
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float MoveSpeed = 100.0f;
    
    UPROPERTY()
    APathSpline* CurrentPath;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float CurrentDistanceAlongSpline;
    
    // 阻挡状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsBlocked;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TWeakObjectPtr<AOperatorBase> BlockedBy;
    
    // 敌人属性
    UPROPERTY(EditDefaultsOnly, Category = "Enemy")
    float MaxHP;
    UPROPERTY(EditDefaultsOnly, Category = "Enemy")
    float ATK;
    UPROPERTY(EditDefaultsOnly, Category = "Enemy")
    float DEF;
    UPROPERTY(EditDefaultsOnly, Category = "Enemy")
    float RES;
    UPROPERTY(EditDefaultsOnly, Category = "Enemy")
    int32 KillCost = 1;
    
    // 开始沿路径移动
    UFUNCTION(BlueprintCallable)
    void StartPathFollowing(APathSpline* Path);
    
    // 被阻挡
    UFUNCTION(BlueprintCallable)
    void OnBlocked(AOperatorBase* Blocker);
    
    // 解除阻挡
    UFUNCTION(BlueprintCallable)
    void OnUnblocked();
    
protected:
    virtual void Tick(float DeltaTime) override;
    
private:
    void UpdatePathFollowing(float DeltaTime);
    void CheckBlocking();
};
```

#### 波次管理器

```cpp
// Enemy/WaveManager.h
USTRUCT(BlueprintType)
struct FWaveEnemyEntry
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<AEnemyBase> EnemyClass;
    
    UPROPERTY(EditDefaultsOnly)
    float SpawnDelay;          // 距离波次开始的延迟
    
    UPROPERTY(EditDefaultsOnly)
    int32 PathIndex;           // 使用哪条路径
};

USTRUCT(BlueprintType)
struct FWaveData
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly)
    TArray<FWaveEnemyEntry> Enemies;
    
    UPROPERTY(EditDefaultsOnly)
    float WaveStartDelay;      // 距离上一波结束的延迟
};

UCLASS()
class AWaveManager : public AActor
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditDefaultsOnly, Category = "Waves")
    TArray<FWaveData> WaveConfigs;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 CurrentWaveIndex;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 TotalEnemiesSpawned;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 TotalEnemiesKilled;
    
    UFUNCTION(BlueprintCallable)
    void StartAllWaves();
    
    UFUNCTION(BlueprintCallable)
    void SpawnWave(int32 WaveIndex);
    
private:
    void SpawnEnemy(TSubclassOf<AEnemyBase> EnemyClass, int32 PathIndex);
};
```

### 3\.3 核心代码实现

#### Spline 路径跟随

```cpp
void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsBlocked && CurrentPath)
    {
        UpdatePathFollowing(DeltaTime);
    }
}

void AEnemyBase::UpdatePathFollowing(float DeltaTime)
{
    // 沿Spline前进
    CurrentDistanceAlongSpline += MoveSpeed * DeltaTime;
    
    // 检查是否到达终点
    if (CurrentDistanceAlongSpline >= CurrentPath->GetPathLength())
    {
        OnReachEndPoint();
        return;
    }
    
    // 获取Spline上的位置和旋转
    FVector TargetLocation = CurrentPath->GetLocationAtDistance(CurrentDistanceAlongSpline);
    FRotator TargetRotation = CurrentPath->GetRotationAtDistance(CurrentDistanceAlongSpline);
    
    // 平滑移动
    SetActorLocation(FMath::VInterpTo(GetActorLocation(), TargetLocation, DeltaTime, 10.0f));
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f));
    
    // 检查阻挡
    CheckBlocking();
}
```

#### 阻挡检测机制

```cpp
void AEnemyBase::CheckBlocking()
{
    FIntPoint CurrentGridCoord = GridManager->WorldToGrid(GetActorLocation());
    FGridCell* CurrentCell = GridManager->GetCellByCoordinate(CurrentGridCoord);
    
    if (CurrentCell && CurrentCell->OccupiedOperator.IsValid())
    {
        AOperatorBase* Blocker = CurrentCell->OccupiedOperator.Get();
        
        // 检查阻挡数是否已满
        if (Blocker->GetCurrentBlockCount() < Blocker->GetMaxBlockCount())
        {
            OnBlocked(Blocker);
            Blocker->AddBlockedEnemy(this);
        }
    }
}

void AEnemyBase::OnBlocked(AOperatorBase* Blocker)
{
    bIsBlocked = true;
    BlockedBy = Blocker;
    
    // 进入攻击状态
    GetWorld()->GetTimerManager().SetTimer(
        AttackTimerHandle,
        this,
        &AEnemyBase::PerformAttack,
        AttackInterval,
        true
    );
}
```

#### 波次生成系统

```cpp
void AWaveManager::SpawnWave(int32 WaveIndex)
{
    if (WaveIndex >= WaveConfigs.Num()) return;
    
    const FWaveData& WaveData = WaveConfigs[WaveIndex];
    
    for (const FWaveEnemyEntry& Entry : WaveData.Enemies)
    {
        // 延迟生成每个敌人
        FTimerHandle SpawnTimer;
        GetWorld()->GetTimerManager().SetTimer(
            SpawnTimer,
            [this, Entry]()
            {
                SpawnEnemy(Entry.EnemyClass, Entry.PathIndex);
            },
            Entry.SpawnDelay,
            false
        );
    }
}

void AWaveManager::SpawnEnemy(TSubclassOf<AEnemyBase> EnemyClass, int32 PathIndex)
{
    APathSpline* Path = PathSplineArray[PathIndex];
    
    FActorSpawnParameters Params;
    AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(
        EnemyClass,
        Path->GetLocationAtDistance(0),
        Path->GetRotationAtDistance(0),
        Params
    );
    
    NewEnemy->StartPathFollowing(Path);
    TotalEnemiesSpawned++;
}
```

### 3\.4 蓝图实现要点

1. **Spline 路径编辑**

    - 在关卡中放置 `BP_PathSpline` Actor

    - 选中 Spline 组件，按住 Alt 点击添加路径点

    - 调整曲线切线实现平滑转弯

2. **敌人状态机（StateTree）**

    ```Plain Text
    StateTree States:
    ├─ Moving (沿Spline移动)
    │  └─ Transition: OnBlocked → Attacking
    ├─ Attacking (攻击阻挡干员)
    │  └─ Transition: OnUnblocked → Moving
    └─ Dead (死亡动画)
    ```

3. **波次可视化**

    - 创建编辑器 Utility Widget 显示波次时间线

    - 支持拖拽调整敌人生成时间点

### 3\.5 注意事项

- **Spline 精度**：敌人密集时使用距离偏移避免重叠（每个敌人偏移 0\.5 个身位）

- **性能优化**：大量敌人时使用 `ISMPawn` 或 Niagara 替代 Character

- **寻路回退**：Spline 断裂时回退到 NavMesh 寻路

- **空中单位**：飞行敌人使用独立 Spline，不受地面阻挡影响

---

## 4\. 攻击系统

### 4\.1 设计思路

明日方舟攻击系统核心机制：

- 攻击范围矩阵（相对坐标定义）

- 锁定目标优先级排序

- 伤害计算公式（物理 / 法术双乘区）

- 攻击前摇 / 后摇 \+ 弹道系统

- 多目标锁定机制

### 4\.2 数据结构定义

#### 攻击范围定义

```cpp
// Combat/AttackRange.h
USTRUCT(BlueprintType)
struct FAttackRangeCell
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly)
    int32 DeltaX;
    
    UPROPERTY(EditDefaultsOnly)
    int32 DeltaY;
    
    UPROPERTY(EditDefaultsOnly)
    bool bCanAttackGround;
    
    UPROPERTY(EditDefaultsOnly)
    bool bCanAttackAir;
};

// 标准攻击范围模板
namespace AttackRangeTemplates
{
    // 近战1格
    static TArray<FAttackRangeCell> Melee_1x1 = {
        {0, 0, true, false}
    };
    
    // 狙击标准范围（向前3格）
    static TArray<FAttackRangeCell> Sniper_Standard = {
        {0, -1, true, true},
        {0, -2, true, true},
        {0, -3, true, true}
    };
    
    // 群攻术师范围
    static TArray<FAttackRangeCell> Caster_AoE = {
        {-1, -1, true, true}, {0, -1, true, true}, {1, -1, true, true},
        {-1, -2, true, true}, {0, -2, true, true}, {1, -2, true, true}
    };
}
```

#### 目标选择数据

```cpp
// Combat/TargetSelector.h
UENUM(BlueprintType)
enum class ETargetPriority : uint8
{
    NEAREST,            // 最近
    FARTHEST,           // 最远
    LOWEST_HP,          // HP最低
    HIGHEST_HP,         // HP最高
    LOWEST_DEF,         // 防御最低
    HIGHEST_WEIGHT,     // 重量最高（优先精英）
    FIRST_BLOCKED       // 最先被阻挡
};

USTRUCT(BlueprintType)
struct FTargetInfo
{
    GENERATED_BODY()
    
    TWeakObjectPtr<AEnemyBase> Enemy;
    float Distance;
    float HP;
    float DEF;
    int32 Weight;
    bool bIsBlocked;
};
```

#### GAS 伤害执行

```cpp
// Combat/GameplayEffect_Damage.h
UCLASS()
class UGameplayEffect_Damage : public UGameplayEffect
{
    GENERATED_BODY()
    
public:
    UGameplayEffect_Damage();
    
    // 伤害类型
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    EDamageType DamageType;
    
    // 计算最终伤害
    static float CalculateDamage(float BaseATK, float TargetDEF, float TargetRES, EDamageType Type);
};
```

### 4\.3 核心代码实现

#### 攻击范围旋转变换

```cpp
TArray<FIntPoint> AOperatorBase::GetRotatedAttackRange(EDirection Direction)
{
    TArray<FIntPoint> Result;
    
    for (const FAttackRangeCell& Cell : OperatorData.AttackRange)
    {
        FIntPoint Rotated = RotateCoordinate(Cell.DeltaX, Cell.DeltaY, Direction);
        Result.Add(Rotated + GridCoordinate);
    }
    
    return Result;
}

FIntPoint AOperatorBase::RotateCoordinate(int32 X, int32 Y, EDirection Direction)
{
    // 方向: 0=上, 1=右, 2=下, 3=左
    switch (Direction)
    {
        case EDirection::UP:    return FIntPoint(X, Y);
        case EDirection::RIGHT: return FIntPoint(-Y, X);
        case EDirection::DOWN:  return FIntPoint(-X, -Y);
        case EDirection::LEFT:  return FIntPoint(Y, -X);
        default: return FIntPoint(X, Y);
    }
}
```

#### 目标搜索与排序

```cpp
TArray<AEnemyBase*> AOperatorBase::FindTargetsInRange()
{
    TArray<FIntPoint> RangeCells = GetRotatedAttackRange(DeployDirection);
    TArray<FTargetInfo> FoundTargets;
    
    // 1. 收集范围内所有敌人
    for (FIntPoint CellCoord : RangeCells)
    {
        FGridCell* Cell = GridManager->GetCellByCoordinate(CellCoord);
        if (!Cell) continue;
        
        for (TWeakObjectPtr<AEnemyBase> EnemyPtr : Cell->BlockingEnemies)
        {
            if (EnemyPtr.IsValid() && EnemyPtr->IsAlive())
            {
                FTargetInfo Info;
                Info.Enemy = EnemyPtr;
                Info.Distance = FVector::Dist(GetActorLocation(), EnemyPtr->GetActorLocation());
                Info.HP = EnemyPtr->GetCurrentHP();
                Info.DEF = EnemyPtr->GetDEF();
                Info.Weight = EnemyPtr->GetEnemyWeight();
                Info.bIsBlocked = EnemyPtr->IsBlocked();
                
                FoundTargets.Add(Info);
            }
        }
    }
    
    // 2. 按优先级排序
    SortTargetsByPriority(FoundTargets, TargetPriority);
    
    // 3. 返回前N个目标（受攻击目标数限制）
    TArray<AEnemyBase*> Result;
    int32 MaxTargets = GetMaxAttackTargetCount();
    
    for (int32 i = 0; i < FMath::Min(FoundTargets.Num(), MaxTargets); i++)
    {
        Result.Add(FoundTargets[i].Enemy.Get());
    }
    
    return Result;
}

void AOperatorBase::SortTargetsByPriority(TArray<FTargetInfo>& Targets, ETargetPriority Priority)
{
    Targets.Sort([Priority](const FTargetInfo& A, const FTargetInfo& B)
    {
        switch (Priority)
        {
            case ETargetPriority::NEAREST:
                return A.Distance < B.Distance;
            case ETargetPriority::LOWEST_HP:
                return A.HP < B.HP;
            case ETargetPriority::HIGHEST_WEIGHT:
                return A.Weight > B.Weight;
            default:
                return A.Distance < B.Distance;
        }
    });
}
```

#### 伤害计算公式

```cpp
float UGameplayEffect_Damage::CalculateDamage(float BaseATK, float TargetDEF, float TargetRES, EDamageType Type)
{
    if (Type == EDamageType::PHYSICAL)
    {
        // 物理伤害: ATK - DEF，最低5%伤害
        float Damage = BaseATK - TargetDEF;
        float MinDamage = BaseATK * 0.05f;
        return FMath::Max(Damage, MinDamage);
    }
    else // EDamageType::MAGICAL
    {
        // 法术伤害: ATK * (1 - RES/100)
        float ResistanceFactor = 1.0f - (TargetRES / 100.0f);
        return BaseATK * FMath::Max(ResistanceFactor, 0.0f);
    }
}

// GAS 伤害执行
void UGA_Attack::ExecuteDamage(AEnemyBase* Target, float Damage, EDamageType Type)
{
    FGameplayEffectSpecHandle DamageSpec = MakeOutgoingGameplayEffectSpec(
        DamageEffectClass,
        GetAbilityLevel()
    );
    
    // 设置伤害数值
    DamageSpec.Data->SetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag("Damage.Base"),
        Damage
    );
    
    // 应用伤害效果
    ApplyGameplayEffectSpecToTarget(DamageSpec, Target->GetAbilitySystemComponent());
}
```

#### 攻击循环

```cpp
void AOperatorBase::StartAttackLoop()
{
    GetWorld()->GetTimerManager().SetTimer(
        AttackTimerHandle,
        this,
        &AOperatorBase::AttackTick,
        OperatorData.AttackInterval,
        true
    );
}

void AOperatorBase::AttackTick()
{
    TArray<AEnemyBase*> Targets = FindTargetsInRange();
    
    if (Targets.Num() > 0)
    {
        // 播放攻击动画前摇
        PlayAttackMontage();
        
        // 延迟造成伤害（匹配动画命中帧）
        FTimerHandle DamageTimer;
        GetWorld()->GetTimerManager().SetTimer(
            DamageTimer,
            [this, Targets]()
            {
                for (AEnemyBase* Target : Targets)
                {
                    DealDamageToTarget(Target);
                }
            },
            AttackHitFrameDelay,
            false
        );
    }
}
```

### 4\.4 蓝图实现要点

1. **攻击范围可视化**

    - 使用 `InstancedStaticMeshComponent` 渲染范围格子

    - 部署时显示半透明蓝色范围

    - 选中干员时高亮显示攻击范围

2. **弹道系统**

    ```Plain Text
    Projectile Blueprint:
    ├─ Event SpawnProjectile(TargetLocation)
    │  ├─ Set Start Location
    │  ├─ Calculate Arc Trajectory
    │  └─ Lerp Position over Time
    └─ Event OnHit
       ├─ Apply Damage GameplayEffect
       └─ Spawn Impact VFX
    ```

3. **攻击动画蒙太奇**

    - 创建 `AM_Attack` 动画蒙太奇

    - 添加 AnimNotify\_AttackHit 在命中帧

    - 支持不同攻击速度的播放速率缩放

### 4\.5 注意事项

- **浮点数精度**：攻击间隔使用 Timer 而非 Tick 计数，避免累积误差

- **目标锁定一致性**：攻击前摇期间目标死亡需重新锁定

- **伤害同步**：多人游戏中伤害计算在 Server 端执行

- **AOE 伤害**：范围伤害使用 SphereTrace 检测，避免重复命中

---

## 5\. 技能系统

### 5\.1 设计思路

明日方舟技能系统核心机制：

- 三种回复类型：自动回复 / 攻击回复 / 受击回复

- 两种触发类型：手动触发 / 自动触发

- 技能持续时间 \+ 初始 SP \+ 消耗 SP

- GAS GameplayAbility 实现技能逻辑

- 天赋系统（被动常驻效果）

### 5\.2 数据结构定义

#### 技能数据结构

```cpp
// Abilities/SkillData.h
UENUM(BlueprintType)
enum class ESPRecoveryType : uint8
{
    AUTO,           // 自动回复
    ATTACK,         // 攻击回复
    HIT             // 受击回复
};

UENUM(BlueprintType)
enum class ESkillTriggerType : uint8
{
    MANUAL,         // 手动开启
    AUTO            // 自动触发
};

USTRUCT(BlueprintType)
struct FSkillData
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FString SkillName;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    ESPRecoveryType RecoveryType;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    ESkillTriggerType TriggerType;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 InitialSP;        // 初始技力
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 MaxSP;            // 所需技力
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Duration;         // 持续时间（0=立即生效）
    
    // GAS 能力类
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> AbilityClass;
    
    // 技能升级数据
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<float> ATKBonusPerLevel;
};
```

#### GAS AttributeSet

```cpp
// Abilities/OperatorAttributeSet.h
UCLASS()
class UOperatorAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
    
public:
    // 基础属性
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_HP)
    FGameplayAttributeData HP;
    ATTRIBUTE_ACCESSORS(UOperatorAttributeSet, HP)
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHP)
    FGameplayAttributeData MaxHP;
    ATTRIBUTE_ACCESSORS(UOperatorAttributeSet, MaxHP)
    
    UPROPERTY(BlueprintReadOnly, Category = "Attributes")
    FGameplayAttributeData ATK;
    ATTRIBUTE_ACCESSORS(UOperatorAttributeSet, ATK)
    
    // 技力属性
    UPROPERTY(BlueprintReadOnly, Category = "Skill", ReplicatedUsing = OnRep_SkillSP)
    FGameplayAttributeData SkillSP;
    ATTRIBUTE_ACCESSORS(UOperatorAttributeSet, SkillSP)
    
    UPROPERTY(BlueprintReadOnly, Category = "Skill")
    FGameplayAttributeData SkillMaxSP;
    ATTRIBUTE_ACCESSORS(UOperatorAttributeSet, SkillMaxSP)
    
    // 属性变更回调
    UFUNCTION()
    virtual void OnRep_HP(const FGameplayAttributeData& OldHP);
    UFUNCTION()
    virtual void OnRep_SkillSP(const FGameplayAttributeData& OldSP);
};
```

### 5\.3 核心代码实现

#### 技力回复系统

```cpp
void AOperatorBase::InitializeSkillSystem()
{
    // 设置初始SP
    GetAbilitySystemComponent()->SetNumericAttributeBase(
        UOperatorAttributeSet::GetSkillSPAttribute(),
        SkillData.InitialSP
    );
    
    // 自动回复技能
    if (SkillData.RecoveryType == ESPRecoveryType::AUTO)
    {
        GetWorld()->GetTimerManager().SetTimer(
            SPRegenTimer,
            this,
            &AOperatorBase::RegenSkillSP,
            1.0f,  // 每秒回复1点
            true
        );
    }
    // 攻击回复技能：绑定攻击事件
    else if (SkillData.RecoveryType == ESPRecoveryType::ATTACK)
    {
        OnAttackPerformed.AddUObject(this, &AOperatorBase::OnAttackForSP);
    }
    // 受击回复技能：绑定受伤事件
    else if (SkillData.RecoveryType == ESPRecoveryType::HIT)
    {
        OnTakeDamage.AddUObject(this, &AOperatorBase::OnTakeDamageForSP);
    }
}

void AOperatorBase::RegenSkillSP()
{
    if (!bIsSkillActive)
    {
        UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
        float CurrentSP = ASC->GetNumericAttribute(UOperatorAttributeSet::GetSkillSPAttribute());
        float MaxSP = SkillData.MaxSP;
        
        if (CurrentSP < MaxSP)
        {
            ASC->SetNumericAttributeBase(
                UOperatorAttributeSet::GetSkillSPAttribute(),
                FMath::Min(CurrentSP + 1, MaxSP)
            );
            
            // 检查是否可以自动触发
            if (SkillData.TriggerType == ESkillTriggerType::AUTO && 
                CurrentSP + 1 >= MaxSP)
            {
                ActivateSkill();
            }
        }
    }
}
```

#### 技能激活与 GameplayEffect

```cpp
bool AOperatorBase::ActivateSkill()
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    float CurrentSP = ASC->GetNumericAttribute(UOperatorAttributeSet::GetSkillSPAttribute());
    
    // SP检查
    if (CurrentSP < SkillData.MaxSP || bIsSkillActive)
        return false;
    
    // 消耗SP
    ASC->SetNumericAttributeBase(
        UOperatorAttributeSet::GetSkillSPAttribute(),
        CurrentSP - SkillData.MaxSP
    );
    
    // 激活GAS能力
    FGameplayAbilitySpecHandle AbilityHandle = ASC->FindAbilitySpecHandleForClass(
        SkillData.AbilityClass
    );
    
    if (AbilityHandle.IsValid())
    {
        ASC->TryActivateAbility(AbilityHandle);
        bIsSkillActive = true;
        
        // 设置技能结束定时器（如果有持续时间）
        if (SkillData.Duration > 0)
        {
            GetWorld()->GetTimerManager().SetTimer(
                SkillDurationTimer,
                this,
                &AOperatorBase::EndSkill,
                SkillData.Duration,
                false
            );
        }
        else
        {
            // 立即型技能直接结束
            EndSkill();
        }
        
        OnSkillActivated.Broadcast(this);
        return true;
    }
    
    return false;
}
```

#### 技能 GameplayAbility 实现

```cpp
// GA_Skill_AttackUp.cpp
void UGA_Skill_AttackUp::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
        return;
    }
    
    AOperatorBase* Operator = Cast<AOperatorBase>(ActorInfo->OwnerActor);
    if (!Operator) return;
    
    // 创建临时属性加成GE
    FGameplayEffectContextHandle EffectContext = MakeEffectContext(Handle, ActorInfo);
    FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(
        BuffEffectClass,
        GetAbilityLevel()
    );
    
    // 设置攻击力加成
    SpecHandle.Data->SetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag("Buff.ATK.Percent"),
        ATKBonusPercent
    );
    
    // 应用持续效果
    ActiveEffectHandle = ApplyGameplayEffectSpecToOwner(
        Handle, ActorInfo, ActivationInfo, SpecHandle
    );
    
    // 播放技能特效
    PlaySkillVFX(Operator);
}

void UGA_Skill_AttackUp::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled
)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    
    // 移除Buff效果
    if (ActiveEffectHandle.IsValid())
    {
        ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle);
    }
}
```

### 5\.4 蓝图实现要点

1. **技能 GameplayAbility 蓝图**

    ```Plain Text
    GA_SkillTemplate:
    ├─ Event ActivateAbility
    │  ├─ Commit Ability
    │  ├─ Apply GameplayEffect to Self (Buff)
    │  ├─ Play Ability Montage
    │  └─ Spawn VFX at Location
    └─ Event EndAbility
       └─ Remove Active GameplayEffect
    ```

2. **技能 UI 显示**

    - 创建 `WBP_SkillButton` 显示 SP 进度

    - SP 满时按钮高亮闪烁

    - 点击按钮发送 `ActivateSkill` 事件

3. **天赋系统实现**

    - 天赋使用永久 `GameplayEffect`

    - 部署时自动应用到干员

    - 支持条件触发天赋（如 HP 低于 50% 时触发）

### 5\.5 注意事项

- **SP 同步**：技力属性必须 Replicated，确保客户端显示正确

- **技能打断**：干员撤退或死亡时必须调用 `EndAbility` 清理

- **效果堆叠**：使用 GameplayTag 限制相同 Buff 不叠加

- **冷却处理**：再部署时保留 SP 进度，不重置

---

## 6\. 目标选择优先级系统

### 6\.1 设计思路

明日方舟目标选择优先级规则：

1. **基础优先级**：重量 \> 阻挡状态 \> 距离 \> HP

2. **职业特殊规则**：医疗优先 HP 最低，狙击优先空中单位

3. **动态优先级**：技能开启时临时改变优先级

4. **目标锁定保持**：攻击前摇期间不切换目标

### 6\.2 数据结构定义

#### 优先级权重配置

```cpp
// Combat/TargetPriorityConfig.h
USTRUCT(BlueprintType)
struct FPriorityWeights
{
    GENERATED_BODY()
    
    // 权重越高优先级越大
    UPROPERTY(EditDefaultsOnly)
    float Weight_EnemyWeight = 1000.0f;      // 敌人重量（精英优先）
    
    UPROPERTY(EditDefaultsOnly)
    float Weight_Blocked = 500.0f;           // 被阻挡优先
    
    UPROPERTY(EditDefaultsOnly)
    float Weight_Distance = 10.0f;           // 距离（负权重=近优先）
    
    UPROPERTY(EditDefaultsOnly)
    float Weight_HP = 1.0f;                  // HP（负权重=低HP优先）
    
    UPROPERTY(EditDefaultsOnly)
    float Weight_IsAirUnit = 200.0f;         // 空中单位优先（狙击）
    
    UPROPERTY(EditDefaultsOnly)
    float Weight_LowHPPercent = 300.0f;      // 低HP百分比优先（医疗）
};
```

#### 目标筛选器

```cpp
UENUM(BlueprintType)
enum class ETargetFilter : uint8
{
    ALL,                // 所有单位
    GROUND_ONLY,        // 仅地面
    AIR_ONLY,           // 仅空中
    BLOCKED_ONLY,       // 仅被阻挡
    UNBLOCKED_ONLY      // 仅未被阻挡
};

UCLASS()
class UTargetSelector : public UObject
{
    GENERATED_BODY()
    
public:
    // 配置
    UPROPERTY(EditDefaultsOnly)
    FPriorityWeights PriorityWeights;
    
    UPROPERTY(EditDefaultsOnly)
    ETargetFilter TargetFilter;
    
    UPROPERTY(EditDefaultsOnly)
    int32 MaxTargetCount = 1;
    
    // 执行目标选择
    UFUNCTION(BlueprintCallable)
    TArray<AEnemyBase*> SelectTargets(
        const TArray<AEnemyBase*>& Candidates,
        AOperatorBase* Selector
    );
    
private:
    float CalculateTargetScore(const FTargetInfo& Target, AOperatorBase* Selector);
    bool PassesFilter(AEnemyBase* Target, ETargetFilter Filter);
};
```

### 6\.3 核心代码实现

#### 优先级评分算法

```cpp
TArray<AEnemyBase*> UTargetSelector::SelectTargets(
    const TArray<AEnemyBase*>& Candidates,
    AOperatorBase* Selector
)
{
    TArray<TPair<float, AEnemyBase*>> ScoredTargets;
    
    // 1. 筛选并评分
    for (AEnemyBase* Candidate : Candidates)
    {
        if (!PassesFilter(Candidate, TargetFilter))
            continue;
        
        FTargetInfo Info = GetTargetInfo(Candidate, Selector);
        float Score = CalculateTargetScore(Info, Selector);
        
        ScoredTargets.Add(TPair<float, AEnemyBase*>(Score, Candidate));
    }
    
    // 2. 按分数降序排序
    ScoredTargets.Sort([](const TPair<float, AEnemyBase*>& A, 
                         const TPair<float, AEnemyBase*>& B)
    {
        return A.Key > B.Key;
    });
    
    // 3. 返回前N个
    TArray<AEnemyBase*> Result;
    for (int32 i = 0; i < FMath::Min(ScoredTargets.Num(), MaxTargetCount); i++)
    {
        Result.Add(ScoredTargets[i].Value);
    }
    
    return Result;
}

float UTargetSelector::CalculateTargetScore(const FTargetInfo& Target, AOperatorBase* Selector)
{
    float Score = 0.0f;
    
    // 敌人重量（精英怪权重极高）
    Score += Target.Weight * PriorityWeights.Weight_EnemyWeight;
    
    // 被阻挡优先
    if (Target.bIsBlocked)
        Score += PriorityWeights.Weight_Blocked;
    
    // 距离优先（负权重=距离越小分数越高）
    Score -= Target.Distance * PriorityWeights.Weight_Distance;
    
    // HP优先（医疗职业特殊处理）
    if (Selector->GetOperatorClass() == EOperatorClass::MEDIC)
    {
        // 医疗：HP百分比越低分数越高
        float HPPercent = Target.HP / Target.MaxHP;
        Score += (1.0f - HPPercent) * PriorityWeights.Weight_LowHPPercent;
    }
    else
    {
        Score -= Target.HP * PriorityWeights.Weight_HP;
    }
    
    // 狙击优先攻击空中单位
    if (Selector->GetOperatorClass() == EOperatorClass::SNIPER && Target.bIsAirUnit)
    {
        Score += PriorityWeights.Weight_IsAirUnit;
    }
    
    return Score;
}
```

#### 职业特殊规则

```cpp
// 职业默认优先级配置
namespace TargetPriorityPresets
{
    // 重装：优先被阻挡的高重量敌人
    static FPriorityWeights Defender()
    {
        FPriorityWeights W;
        W.Weight_Blocked = 1000.0f;
        W.Weight_EnemyWeight = 800.0f;
        W.Weight_Distance = 5.0f;
        return W;
    }
    
    // 医疗：优先HP最低的友方
    static FPriorityWeights Medic()
    {
        FPriorityWeights W;
        W.Weight_LowHPPercent = 500.0f;
        W.Weight_Distance = 10.0f;
        return W;
    }
    
    // 狙击：优先空中单位
    static FPriorityWeights Sniper()
    {
        FPriorityWeights W;
        W.Weight_IsAirUnit = 500.0f;
        W.Weight_EnemyWeight = 200.0f;
        W.Weight_Distance = 15.0f;
        return W;
    }
    
    // 群攻术师：优先密集区域
    static FPriorityWeights Caster_AoE()
    {
        FPriorityWeights W;
        W.Weight_EnemyWeight = 300.0f;
        W.Weight_Distance = 8.0f;
        return W;
    }
}
```

#### 目标锁定保持机制

```cpp
void AOperatorBase::AttackTick()
{
    // 如果已有锁定目标且仍在范围内，保持锁定
    if (LockedTarget.IsValid() && IsTargetInRange(LockedTarget.Get()))
    {
        PerformAttack(LockedTarget.Get());
        return;
    }
    
    // 否则重新搜索目标
    TArray<AEnemyBase*> NewTargets = TargetSelector->SelectTargets(
        GetAllEnemiesInRange(),
        this
    );
    
    if (NewTargets.Num() > 0)
    {
        LockedTarget = NewTargets[0];
        PerformAttack(LockedTarget.Get());
    }
    else
    {
        LockedTarget.Reset();
    }
}

bool AOperatorBase::IsTargetInRange(AEnemyBase* Target)
{
    FIntPoint TargetCoord = GridManager->WorldToGrid(Target->GetActorLocation());
    TArray<FIntPoint> AttackRange = GetRotatedAttackRange(DeployDirection);
    return AttackRange.Contains(TargetCoord);
}
```

### 6\.4 蓝图实现要点

1. **目标选择调试**

    - 创建调试绘制：用不同颜色显示每个目标的优先级分数

    - 红色 = 最高优先级，黄色 = 中等，绿色 = 最低

2. **技能临时优先级切换**

    ```Plain Text
    Skill Activation:
    ├─ Save Current Priority Config
    ├─ Apply Skill-Specific Priority Weights
    └─ Skill End: Restore Original Config
    ```

3. **目标指示器**

    - 锁定目标上方显示红色瞄准标记

    - 多目标攻击时显示所有锁定目标

### 6\.5 注意事项

- **权重调优**：使用数据表配置权重，避免硬编码

- **性能优化**：目标搜索每 0\.1 秒执行一次，不必每帧

- **确定性**：相同条件下目标选择结果必须一致（多人同步）

- **边界处理**：分数相同时使用 EntityID 作为 tie\-breaker

---

## 总结与集成建议

### 推荐项目结构

```Plain Text
Source/ArkNightsTD/
├── GridSystem/              # 格子系统
│   ├── GridManager.h/cpp
│   └── GridCell.h
├── Deployment/              # 部署系统
│   ├── DeploymentManager.h/cpp
│   └── OperatorPreview.h/cpp
├── Enemy/                   # 敌人系统
│   ├── EnemyBase.h/cpp
│   ├── PathSpline.h/cpp
│   └── WaveManager.h/cpp
├── Operator/                # 干员系统
│   ├── OperatorBase.h/cpp
│   └── OperatorData.h
├── Combat/                  # 战斗系统
│   ├── TargetSelector.h/cpp
│   └── DamageCalculation.h
└── Abilities/               # GAS技能系统
    ├── OperatorAttributeSet.h/cpp
    ├── GA_Attack.h/cpp
    └── GA_Skill_Base.h/cpp
```

### 关键插件依赖

- **GameplayAbilities** \- GAS 核心

- **GameplayTags** \- 标签系统

- **EnhancedInput** \- 输入系统

- **StateTree** \- AI 状态机

- **Niagara** \- 特效系统

### 性能优化建议

1. **对象池**：敌人和弹道使用对象池复用

2. **空间分区**：格子系统天然支持空间查询

3. **批量更新**：同类型干员统一更新攻击计时

4. **LOD 系统**：远处敌人降低更新频率

---

**文档结束**

> （注：文档部分内容可能由 AI 生成）
