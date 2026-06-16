# 代码内容重构计划：对齐明日方舟塔防技术文档

## 执行顺序

执行顺序按**最小依赖 → 最大依赖**排列。**每一步完成后编译+commit**。

---

## Step 1: 攻击范围矩阵系统 (Attack Range Matrix)

### 当前状态
- `TDBaseTower` 使用 `RangeSphere` (USphereComponent) 圆形范围，通过 `FVector::Dist()` 检测敌人
- 没有网格感知的攻击范围概念

### 参考文档目标
- `FAttackRangeCell` 结构体定义相对坐标攻击范围
- 攻击范围支持方向旋转变换（当前保留固定朝前）
- 塔知道自己的网格坐标

### 与动画系统关系
- **❌ 不影响**：`FindTarget()` 仅改变索敌逻辑，不改变动画状态机
- 现有状态机：`Starting→Idle→AttackStarting→Attacking→AttackEnding→Idle→Dying→Destroy` 保持不变
- 所有 `AnimState` 枚举值、`OnAnimComplete` 驱动逻辑均不变

### 改动

**File: Public/Tower/TDAttackRange.h** *(新建)*
```cpp
USTRUCT(BlueprintType)
struct FAttackRangeCell
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly) int32 DeltaX;
    UPROPERTY(EditDefaultsOnly) int32 DeltaY;
};
```

**File: Public/Tower/TDBaseTower.h**
- 添加 `UPROPERTY(EditDefaultsOnly, Category = "Tower|Attack") TArray<FAttackRangeCell> AttackRangeCells;`
- 添加 `int32 GridCol = -1; int32 GridRow = -1;` — 部署时设置的网格坐标
- 添加 `void SetGridCoordinate(int32 Col, int32 Row);`

**File: Private/Tower/TDBaseTower.cpp**
- 修改 `FindTarget()`：
  1. 遍历所有敌人（不变）
  2. 距离检测不变（兼容现有逻辑）
  3. **增加**：若 `GridCol>=0` 且 `AttackRangeCells` 有定义，额外检查敌人格子是否在攻击范围矩阵内
- 构造函数添加默认 `AttackRangeCells = {{0,0}}`（近卫1格）

**File: Private/Core/TDPlayerController.cpp**
- 部署塔后在 `TryOccupy` 旁边添加 `Tower->SetGridCoordinate(Col, Row);`

**依赖**: TDGridManager (已存在)

---

## Step 2: 塔添加 MaxBlockCount 字段

### 当前状态
- 塔没有阻挡概念
- 敌人可以穿过塔的位置

### 改动
**File: Public/Tower/TDBaseTower.h**
- 添加 `UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower|Combat") int32 MaxBlockCount = 1;`
- 添加 `int32 GetCurrentBlockCount() const;`
- 添加 `TArray<TWeakObjectPtr<class ATDEnemy>> BlockedEnemies;`（为 Step 4 预留）
- 添加 `void AddBlockedEnemy(ATDEnemy* Enemy);`
- 添加 `void RemoveBlockedEnemy(ATDEnemy* Enemy);`

**File: Private/Tower/TDBaseTower.cpp**
- 实现 `GetCurrentBlockCount()` → 返回 `BlockedEnemies.Num()`
- 实现 `AddBlockedEnemy/RemoveBlockedEnemy`

### 与动画系统关系
- **❌ 不影响**：仅添加数据字段和简单存取函数

---

## Step 3: 目标选择器接入 (TargetSelector Integration)

### 当前状态
- `TDTargetSelector.h` 已有 `ETargetPriority` 枚举和 `UTDGAntiSelector` 类
- `TDBaseTower::FindTarget()` 硬编码最近距离

### 改动

**File: Public/Tower/TDTargetSelector.h**
- 类名改为 `UTDGTargetSelector`（更一致命名）
- 添加 `SelectTargets(const TArray<ATDEnemy*>& Candidates, ATDBaseTower* Selector)` 声明

**File: Private/Tower/TDTargetSelector.cpp** *(新建)*
- 实现 `SelectTargets()`：
  - `NEAREST`：距离升序
  - `LOWEST_HP`：HP升序
  - `FARTHEST`：距离降序
  - 其他优先级返回排序后的第一个

**File: Public/Tower/TDBaseTower.h**
- 添加 `UPROPERTY(EditDefaultsOnly, Category = "Tower|Combat") TObjectPtr<UTDGTargetSelector> TargetSelector;`

**File: Private/Tower/TDBaseTower.cpp**
- 修改 `FindTarget()`：
  1. 收集攻击范围内所有敌人（同现有逻辑）
  2. 若 `TargetSelector` 存在 → 调用 `SelectTargets`，取第一个结果
  3. 若 `TargetSelector` 为空 → 回退最近距离逻辑（完全兼容）

### 与动画系统关系
- **❌ 不影响**：`FindTarget()` 输出类型不变（`CurrentTarget` 指针），动画状态机无感知

---

## Step 4: 阻挡系统 (Blocking System)

### 当前动画状态机（不变，只参考）
```
敌人状态机:
MoveBeginning(Move_Begin,once)→Moving(Move_Loop,loop)
                                →MoveEnding(Move_End,once)→Attacking(Attack,loop)→MoveBeginning→...

塔状态机:
Starting→Idle→AttackStarting→Attacking→AttackEnding→Idle→Dying→Destroy
```

### 核心设计思路
**阻挡系统不改变现有动画状态机**，只增加一个 `bIsBlocked` 标志位来控制敌人是否沿 Spline 前进：
- `bIsBlocked = true` → 禁止 `DistanceAlongSpline += MoveSpeed * DeltaTime`
- **敌人原有的** `FindNearestTower()` 和距离检测继续保持：检测到塔在近战范围内时，自然触发 → MoveEnding → Attacking
- `bIsBlocked = false` → 恢复 Spline 前进，敌人自然执行 MoveBeginning → Moving → ...

### 与动画系统的边界
- **❌ 不新增** 动画状态枚举值
- **❌ 不修改** 现有状态机变量命名 (`AnimState`, `EEnemyAnimState`, `ETowerAnimState`)
- **❌ 不修改** `OnAnimComplete` 驱动逻辑
- **✅ 只加** `bIsBlocked` 标志位 + 对应的 Spline 移动条件

### 改动

**File: Public/Enemy/TDEnemy.h**
- 添加 `bool bIsBlocked = false;`
- 添加 `TWeakObjectPtr<ATDBaseTower> BlockedByTower;`
- 添加 `void OnBlocked(ATDBaseTower* Blocker);`
- 添加 `void OnUnblocked();`

**File: Public/Tower/TDBaseTower.h**
- `BlockedEnemies`、`MaxBlockCount`、`GetCurrentBlockCount()` 已在 Step 2 添加
- 修改 `Die()`：死亡时释放 `BlockedEnemies`
- 添加 `void FreeAllBlockedEnemies();`

**File: Private/Tower/TDBaseTower.cpp**
- 实现 `FreeAllBlockedEnemies()`：遍历 `BlockedEnemies`，对每个调用 `OnUnblocked()`
- `Die()` 中调用 `FreeAllBlockedEnemies()` 再销毁

**File: Private/Enemy/TDEnemy.cpp**
- 修改 Tick() 中的 Spline 前进逻辑：
  - 原：`if (AnimState == Moving || MoveBeginning)`
  - 改：**额外加** `&& !bIsBlocked`
- 实现 `OnBlocked(Blocker)`：
  - `bIsBlocked = true`
  - `BlockedByTower = Blocker`
  - `Blocker->AddBlockedEnemy(this)`
- 实现 `OnUnblocked()`：
  - `bIsBlocked = false`
  - `BlockedByTower = nullptr`
- 修改 `Tick()` 中的阻挡检测逻辑：
  - **替换**现有的 `FindNearestTower()` + `Dist <= MeleeRange` 逻辑
  - 新逻辑：仅当 `!bIsBlocked` 且未受阻时，检测是否有塔在近战范围内
  - 若塔有阻挡空位（`GetCurrentBlockCount() < MaxBlockCount`）→ `OnBlocked`
  - 若塔无阻挡空位 → 穿过去（不阻挡）
- `Die()` 中：若 `bIsBlocked`，调用 `BlockedByTower->RemoveBlockedEnemy(this)`

---

## Step 5: 波次管理器实现 (WaveManager)

### 当前状态
- 存根，`StartAllWaves()` 和 `SpawnWave()` 为空

### 改动

**File: Public/Enemy/TDWaveManager.h**
- 添加 `UPROPERTY(EditAnywhere, Category = "Paths") TArray<TObjectPtr<AActor>> PathActors;`
- 添加 `void SpawnEnemy(TSubclassOf<ATDEnemy> EnemyClass, int32 PathIndex);`

**File: Private/Enemy/TDWaveManager.cpp**
- 实现 `SpawnWave(int32 WaveIndex)`：
  - 从 `WaveConfigs[WaveIndex]` 读取 `Enemies` 数组
  - 为每个条目创建延迟定时器 → 到期调用 `SpawnEnemy`
- 实现 `SpawnEnemy(EnemyClass, PathIndex)`：
  - 在 `PathActors[PathIndex]` 的 Spline 起点生成敌人
  - 设置敌人的 `PathActor`
- 实现 `StartAllWaves()`：
  - 从 `CurrentWaveIndex = 0` 开始
  - 考虑 `WaveStartDelay`

### 与动画系统关系
- **❌ 不影响**：波次管理器只负责生成敌人，生成后敌人自然使用自己的动画状态机

---

## 文件变更总览

| Step | 新建 | 修改 | 动画影响 |
|------|------|------|---------|
| 1 | `Tower/TDAttackRange.h` | `TDBaseTower` + `TDPlayerController` | ❌ 无 |
| 2 | — | `TDBaseTower` | ❌ 无 |
| 3 | `Tower/TDTargetSelector.cpp` | `TDTargetSelector.h` + `TDBaseTower` | ❌ 无 |
| 4 | — | `TDEnemy` + `TDBaseTower` | ✅ 不改变状态机，加 `bIsBlocked` 标志位 |
| 5 | — | `TDWaveManager` | ❌ 无 |

---

## 关键约定
1. **不修改**现有状态机枚举值命名（`EEnemyAnimState`, `ETowerAnimState`, `AnimState`）
2. **不修改**现有 `OnAnimComplete` 和状态机驱动逻辑
3. **不修改**动画名称字符串（`Move_Begin`, `Move_Loop`, `Attack` 等）
4. 阻挡系统只加 `bIsBlocked` 标志位，不影响动画触发逻辑
5. 每次 commit 前确保编译通过
