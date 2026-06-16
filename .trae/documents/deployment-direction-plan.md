# 部署方向系统 + 拖拽部署 实现计划

## 方向选择机制

**不按R键循环方向，而是根据鼠标在格子上的位置判断方向：**

鼠标在格子上的位置 → 计算相对于格子中心的偏移(dx, dy)
- `|dx| > |dy|` → `dx > 0` 为 UP(朝上), `dx < 0` 为 DOWN(朝下)
- `|dy| >= |dx|` → `dy > 0` 为 RIGHT(朝右), `dy < 0` 为 LEFT(朝左)

玩家视角: X+为屏幕上方, Y+为屏幕右方

## 执行步骤

### Step A: EDirection 枚举 (新建)

**文件: Public/Tower/TDDeployDirection.h**
```cpp
UENUM(BlueprintType)
enum class EDirection : uint8
{
    RIGHT   UMETA(DisplayName = "朝右"),
    LEFT    UMETA(DisplayName = "朝左"),
    UP      UMETA(DisplayName = "朝上"),
    DOWN    UMETA(DisplayName = "朝下")
};
```

### Step B: TDBaseTower 方向支持

**文件: Public/Tower/TDBaseTower.h**
- 添加 `SpineAnimBack` (第二个 Spine 组件, 用于 UP 方向)
- 添加 `EDirection DeployDirection = EDirection::RIGHT`
- 添加 `void SetDeployDirection(EDirection NewDir)`

**文件: Private/Tower/TDBaseTower.cpp**
- 构造函数: 创建 `SpineAnimBack` 组件
- `SetDeployDirection()`:
  - RIGHT: SpineAnim显示, SpineAnimBack隐藏, Scale.Y=1(朝右→Y+)
  - LEFT: SpineAnim显示, Scale.Y=-1(镜像朝左→Y-), SpineAnimBack隐藏
  - UP: SpineAnim隐藏, SpineAnimBack显示, Scale=1
  - DOWN: SpineAnim显示, Scale.Y=-1(同LEFT), SpineAnimBack隐藏

### Step C: 攻击范围方向旋转

**文件: Private/Tower/TDBaseTower.cpp**
- 修改 `IsEnemyInRangeCells()`: 根据 DeployDirection 旋转相对坐标
- 旋转公式:
  - RIGHT: (dx, dy) 不变
  - LEFT: (dx, -dy)
  - UP: (dy, dx)
  - DOWN: (-dy, dx)

### Step D: PlayerController 鼠标位置决定方向

**文件: Public/Core/TDPlayerController.h**
- 添加 `EDirection GetDirectionFromMouse(FVector GridWorldCenter) const`

**文件: Private/Core/TDPlayerController.cpp**
- 修改 `UpdatePreview()`: 计算鼠标相对于格子中心的方向
- 修改 `OnClick()`: 部署时传入方向 `Tower->SetDeployDirection(CurrentDirection)`

### Step E: 手牌UI + 拖拽系统

创建手牌 Widget 蓝图 + C++ 桥接接口:
1. `TDPlayerController` 添加 `HandCards` 数组
2. 拖拽时设置 TowerToDeploy, 鼠标位置决定方向
3. 松手部署

## 关键坐标映射

```
鼠标在格子上的位置:
      ↑ X+ (UP)
      |
  Y- ← ○ → Y+ (RIGHT)    ○ = 格子中心
  (LEFT) |
      ↓ X- (DOWN)
```

方向判断:
```
dx = MouseWorld.X - CellCenter.X
dy = MouseWorld.Y - CellCenter.Y
if |dx| > |dy| → dx>0 ? UP : DOWN
else           → dy>0 ? RIGHT : LEFT
```

## 执行顺序

1. Step A: EDirection 枚举
2. Step B: TDBaseTower SetDeployDirection + SpineAnimBack
3. Step C: 攻击范围旋转
4. Step D: PlayerController 方向计算
5. Step E: 手牌UI + 拖拽 (小范围先做C++接口, 蓝图部分后续)
