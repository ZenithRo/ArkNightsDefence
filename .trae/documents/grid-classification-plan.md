# 格子分类系统 + 可视化编辑器 实现计划

## 执行步骤

### Step 1: C++ 数据结构改造

**1a. 扩展 ETileType / FGridCellData**
- `ETileType` 添加 `HOLE = 1 << 5`（地穴死亡格）
- `FGridCellData` 保持 `bDeployable` 字段（兼容旧代码），但部署逻辑改为优先检查 `TileType`

**1b. 塔添加部署类型标识**
- `Public/Tower/TDBaseTower.h`：
  ```
  UENUM(BlueprintType)
  enum class ETowerPlacement : uint8
  {
      GROUND_ONLY   UMETA(DisplayName = "仅地面"),
      HIGHLAND_ONLY UMETA(DisplayName = "仅高台"),
      ANY           UMETA(DisplayName = "均可")
  };
  ```
- 添加 `UPROPERTY(EditDefaultsOnly) ETowerPlacement PlacementType = ETowerPlacement::GROUND_ONLY;`

**1c. 更新 CanDeployAt 逻辑**
- `TDGridManager::CanDeployAt()` 改为接收 `ETowerPlacement` 参数
- 或新建 `CanDeployAt(int32 Col, int32 Row, ETowerPlacement Placement) const`
  - HOLE → 永远不可部署
  - BLOCKED → 永远不可部署
  - GROUND → 仅 GROUND_ONLY 或 ANY 可部署
  - HIGHLAND → 仅 HIGHLAND_ONLY 或 ANY 可部署

**1d. ApplyToGridManager 补传 TileType**
- `TDGridDataActor::ApplyToGridManager()` 现在也复制 `TileType` 字段

### Step 2: TDGridDataActor 可视化增强

- `DrawEditorGrid()` 改为按 TileType 着色：
  - GROUND → 浅绿
  - HIGHLAND → 深蓝
  - BLOCKED → 灰色
  - HOLE → 深红（并绘制 90% 死亡判定矩形线框）
- `DrawHeight` 用于所有格子的 Z 高度
- 额外在 HOLE 格子中心画一个 90% 大小的 `DrawDebugBox`

### Step 3: GridDataAsset 数据持久化

**新建文件: Public/Grid/TDGridDataAsset.h**
```cpp
UCLASS()
class UTDGridDataAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) int32 NumCols = 10;
    UPROPERTY(EditAnywhere) int32 NumRows = 8;
    UPROPERTY(EditAnywhere) float CellSize = 200.0f;
    UPROPERTY(EditAnywhere) TArray<FGridCellData> Cells;
};
```

- `TDGridDataActor` 添加 `UPROPERTY(EditAnywhere) TObjectPtr<UTDGridDataAsset> GridDataAsset;`
- `ApplyToGridManager()` 新增从 DataAsset 加载数据的分支
- 支持从 DataAsset 导入/导出到 `Cells` 数组

### Step 4: HOLE死亡检测运行时逻辑

- `TDGridManager` 添加 `bool IsHoleCell(int32 Col, int32 Row) const` 和 `FBox2D GetHoleDeathBox(int32 Col, int32 Row) const`
- 在 `TDEnemy::Tick()` 中每帧检测：若敌人所在格子是 HOLE，且位置在 90% 死亡框内 → `Die()`

### Step 5: Editor Utility Widget (蓝图)

**C++ 侧准备接口：**
- `TDGridDataActor` 添加蓝图可调用的：
  - `GetCellInfo(int32 Col, int32 Row) → FGridCellData`
  - `SetCellType(int32 Col, int32 Row, ETileType NewType)`
  - `SetGridSize(int32 Cols, int32 Rows)`
  - `ExportToJSON() → FString`
  - `ImportFromJSON(FString JSON)`
  - `SaveToDataAsset()`
  - `LoadFromDataAsset(UTDGridDataAsset* Asset)`

**编辑器蓝图（Editor Utility Widget）：**
1. 创建 `WBP_GridEditor` (Editor Utility Widget)
2. 左侧面板：笔刷选择按钮 (地面/高台/阻挡/地穴) + 键盘快捷键 1/2/3/4
3. 右侧参数面板：Cols/Rows/CellSize/GridOrigin 实时调整
4. 视口覆盖：鼠标点击→单格编辑，拖拽→批量涂抹
5. 数据操作按钮：导出JSON/导入JSON/保存到DataAsset/从DataAsset加载

## 执行顺序

1. C++ 数据结构 (Step 1a-1d): ETileType扩展 + TowerPlacement + CanDeployAt + ApplyToGridManager
2. 可视化增强 (Step 2): DrawEditorGrid 四色 + HOLE线框
3. HOLE运行时逻辑 (Step 4): IsHoleCell + 死亡检测
4. GridDataAsset (Step 3): DataAsset类 + TDGridDataActor集成
5. 编辑器接口 (蓝图, Step 5): E UW + JSON导入导出
