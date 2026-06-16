# 部署系统设计文档

## 概述

为塔防游戏实现基于格子的部署系统，通过双射线检测精准定位塔的放置位置，支持高低地形贴合，并提供编辑器可视化工具用于人工编辑格子数据。

## 架构组件

### 1. 碰撞通道（Collision Channels）

在 `DefaultEngine.ini` 中新增两个自定义 **Trace Channel**：

| Channel Name | 用途 | 默认响应 |
|-------------|------|---------|
| `DeploymentPlane` | 高空判定平面，用于获取 XOY 平面坐标 | Ignore |
| `MapMesh` | 地图地形网格，用于采样地形高度 | Ignore |

**碰撞矩阵：**

| 对象 | DeploymentPlane | MapMesh | Visibility |
|------|----------------|---------|------------|
| 判定平面 Mesh | **Block** | Ignore | Ignore |
| 地图地形 Mesh | Ignore | **Block** | Block |
| 塔/敌人等 | Ignore | Ignore | Block |

### 2. TDGridManager（核心模块）

**类型：** `UObject`，全局单例，通过 GameInstance 或 GameMode 访问。

**数据结构：**

```
FGridCell {
    bool bDeployable;    // 是否允许部署
    bool bOccupied;      // 是否已被塔占用
}
```

- 存储 `NumCols × NumRows` 二维数组（默认 10×8）
- 格子大小：`CellSize = 200`
- 原点：网格左下角世界坐标 `GridOrigin`

**核心接口：**

```cpp
// 世界坐标 ↔ 网格坐标
bool WorldToGrid(FVector WorldPos, int32& OutCol, int32& OutRow) const;
FVector GridToWorld(int32 Col, int32 Row) const;  // 返回格子中心世界坐标

// 合法性校验
bool CanDeployAt(int32 Col, int32 Row) const;     // 是否可部署
bool TryOccupy(int32 Col, int32 Row);              // 标记占用
void Free(int32 Col, int32 Row);                   // 解除占用

// 双射线检测
bool GetDeployLocation(FVector2D ScreenPos, FVector& OutLocation, int32& OutCol, int32& OutRow);

// 编辑器接口
void SetDeployable(int32 Col, int32 Row, bool bDeployable);
```

### 3. 判定平面（DeploymentPlane）

- 在关卡中放置一个静态网格体（`Plane`）
- 调整到高空（如 Z = 5000）
- 碰撞预设：只阻挡 `DeploymentPlane` Trace Channel
- 材质：透明不可见

### 4. 双射线检测逻辑

**第一层射线（平面定位）：**
```
从摄像机 → 穿过地形 → 命中判定平面
Channel: DeploymentPlane
忽略: MapMesh, Visibility
输出: Hit.Location (X, Y, Z=PlaneZ)
```

**坐标换算：**
```
Col = (Hit.X - GridOrigin.X) / CellSize
Row = (Hit.Y - GridOrigin.Y) / CellSize
→ 取整得到格子索引
```

**第二层射线（地形对齐）：**
```
从格子中心 (GridCenter.X, GridCenter.Y, HighZ) 垂直向下
Channel: MapMesh
忽略: DeploymentPlane
输出: Hit.Location.Z → 地形表面高度
```

**最终部署位置：**
```
SpawnLocation = (GridCenter.X, GridCenter.Y, TerrainZ)
```

### 5. TDDeploymentPreviewActor

**类型：** `AActor`

**功能：**
- 实时半透明塔预览，跟随鼠标在网格上移动
- 合法位置显示绿色，非法位置显示红色
- 鼠标点击时触发部署
- 在玩家 Controller 中管理其生命周期

### 6. TDGridDataActor（编辑器工具）

**类型：** `AActor`，仅编辑器生效

**功能：**
- 放置到关卡中，关联一个 `TDGridManager` 引用
- 视口 Debug 绘制：
  - 绿色方框：可部署且未占用
  - 红色方框：不可部署
  - 灰色方框：已占用
- 属性面板中暴露 `GridCells` 二维数组，每个格子有 `bDeployable` 开关
- 运行时自动隐藏

### 7. 修改 ATDPlayerController

**OnClick 新流程：**
1. 获取 `TDGridManager` 单例
2. 调用 `GetDeployLocation(MousePos, OutLocation, OutCol, OutRow)`
3. 若失败，拒绝部署
4. 调用 `CanDeployAt(Col, Row)` 检查合法性
5. 调用 `GM->SpendCost(ToDeployCost)` 扣费
6. 在 `OutLocation` 生成塔
7. 调用 `TryOccupy(Col, Row)` 标记占用

## 实现顺序

1. DefaultEngine.ini 配置碰撞通道
2. TDGridManager（核心数据结构 + 双射线检测）
3. 判定平面（关卡放置 + Collision Preset）
4. TDDeploymentPreviewActor（预览 + 交互）
5. 修改 ATDPlayerController（对接新部署逻辑）
6. TDGridDataActor（编辑器可视化编辑工具）
7. Test & Polish
