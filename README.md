# ArkNightsDefence--一个仿ArkNights的塔防小游戏--基于UE5的C++大作业项目

### 5/26 项目立项

### 第1周 (5/27-5/29) 基础框架搭建

#### C++ 类

| 文件 | 说明 |
|------|------|
| `Source/ArkNightsDefence/Public/TDTopDownPawn.h` | Pawn头文件：继承APawn，声明SpringArm/Camera组件，MoveAction/ZoomAction输入引用，CameraPitch/CloseZoom/FarZoom/MoveSpeed可配置参数 |
| `Source/ArkNightsDefence/Private/TDTopDownPawn.cpp` | 构造函数创建USpringArmComponent（bDoCollisionTest=false、bInheritPitch/Roll=false）+UCameraComponent；BeginPlay用CameraPitch设置俯角；Move()基于摄像机方向WASD水平平移；Zoom()根据滚轮方向切换CloseZoom/FarZoom |
| `Source/ArkNightsDefence/Public/TDPlayerController.h` | PlayerController头文件：继承APlayerController，声明DefaultMappingContext/ClickAction，重写SetupInputComponent()，OnClick()鼠标点击响应 |
| `Source/ArkNightsDefence/Private/TDPlayerController.cpp` | BeginPlay显示鼠标光标+注册EnhancedInput；SetupInputComponent绑定ClickAction(Started事件)；OnClick()用GetHitResultUnderCursor射线检测点击位置，Debug红球+打印坐标 |
| `Source/ArkNightsDefence/ArkNightsDefence.Build.cs` | 构建配置：引入Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG |
| `Source/ArkNightsDefence/ArkNightsDefence.h/.cpp` | 模块入口文件 |
| `Source/ArkNightsDefence.Target.cs / ArkNightsDefenceEditor.Target.cs` | 编译目标配置 |

#### 蓝图资产

| 蓝图 | 类型 | 说明 |
|------|------|------|
| `BP_TDTopDownPawn` | 蓝图类（父类TDTopDownPawn） | CameraPitch=-60, CloseZoom=800, FarZoom=2000, MoveSpeed=1500 |
| `BP_TDPlayerController` | 蓝图类（父类TDPlayerController） | DefaultMappingContext→IMC_TDGameplay, ClickAction→IA_Click |
| `BP_TDGameMode` | 蓝图类（父类GameModeBase） | DefaultPawnClass→BP_TDTopDownPawn, PlayerControllerClass→BP_TDPlayerController |
| `IMC_TDGameplay` | Input Mapping Context | W/A/S/D→IA_Move(2D Swizzle+Negate), MouseWheel→IA_Zoom, LeftMouse→IA_Click |
| `IA_Move` | Input Action (Axis2D) | WASD四向移动 |
| `IA_Zoom` | Input Action (Axis1D) | 滚轮缩放（前滚=CloseZoom, 后滚=FarZoom） |
| `IA_Click` | Input Action (Digital) | 鼠标左键点击 |

#### 配置文件

| 文件 | 说明 |
|------|------|
| `Config/DefaultInput.ini` | EnhancedInput系统配置 |
| `.gitignore` | 忽略.vs, Binaries, Intermediate, DerivedDataCache, Saved, UpgradeLog.htm, week1_progress.txt, week2_progress.txt |

#### 功能验证

- ✅ 俯视角摄像机（CameraPitch可调，默认-60°）
- ✅ WASD四向平移（基于摄像机方向，帧率无关）
- ✅ 滚轮缩放（前滚=拉近CloseZoom，后滚=拉远FarZoom）
- ✅ 鼠标左键点击地面 → 射线检测 + Debug红球 + 坐标打印
- ✅ ClassDefaults面板可调所有Camera参数

---

### 第2周 (5/31-6/2) 敌人系统 + Spline路径 + GameMode + UMG HUD

#### C++ 类

| 文件 | 说明 |
|------|------|
| `Source/ArkNightsDefence/Public/TDEnemy.h` | 敌人基类：EDamageType枚举(Physical/Magic)，Sphere碰撞体+StaticMesh组件，MaxHealth/MoveSpeed/PhysicalArmor/MagicResistance/ExperienceDrop/LifeDamage，PathActor引用BP_Path，ApplyDamage(Amount, Type)双防减伤，Die()/OnReachedEnd() |
| `Source/ArkNightsDefence/Private/TDEnemy.cpp` | BeginPlay缓存Spline。Tick沿路径推进+旋转+终点检测。ApplyDamage: 物理=伤害-PHYS_DEF, 法术=伤害×(1-RES%), 均≥1。Die掉落经验，OnReachedEnd扣生命 |
| `Source/ArkNightsDefence/Public/TDGameMode.h` | GameMode：PlayerLives(3)，双货币——Cost(初始0, MaxCost=99, Timer每秒+1.0) + Experience，HUDWidget引用，EnemyReachedEnd/AddExperience/SpendCost含Debug打印+自动刷新HUD |
| `Source/ArkNightsDefence/Private/TDGameMode.cpp` | BeginPlay启动CostRegenTimer(1s)。RegenerateCost/EnemyReachedEnd/AddExperience/SpendCost均调用HUDWidget->UpdateDisplay()事件驱动刷新 |
| `Source/ArkNightsDefence/Public/TDHUDWidget.h` | UMG HUD Widget: 事件驱动更新(无Tick)，UFUNCTION UpdateDisplay()，BindWidget绑定TextLives/TextCost/TextExp |
| `Source/ArkNightsDefence/Private/TDHUDWidget.cpp` | UpdateDisplay从GameMode读取PlayerLives/Cost/Experience刷新TextBlock |
| `Source/ArkNightsDefence/Public/TDTopDownPawn.h` | 新增ZoomStep/ZoomInterpSpeed参数，TargetZoomDistance私有成员，Tick平滑插值弹簧臂长度 |
| `Source/ArkNightsDefence/Private/TDTopDownPawn.cpp` | Tick中FMath::FInterpTo平滑缩放；Zoom改为步进(±ZoomStep)钳制在CloseZoom~FarZoom之间 |

#### 蓝图资产

| 蓝图 | 类型 | 说明 |
|------|------|------|
| `BP_Path` | 蓝图类（父类Actor） | 含USplineComponent(Root)，关卡中画敌人路径 |
| `BP_Enemy_Infantry` | 蓝图类（父类TDEnemy） | 步兵：80HP/400速度/0物防/0法抗/5经验/1生命伤害 |
| `BP_Enemy_Armored` | 蓝图类（父类TDEnemy） | 装甲兵：300HP/150速度/10物防/0法抗/15经验/2生命伤害 |
| `BP_Enemy_Flying` | 蓝图类（父类TDEnemy） | 飞行单位：120HP/250速度/0物防/0法抗/10经验/1生命伤害 |
| `M_Red` / `M_Blue` | 材质 | 敌人颜色区分（步兵=绿，装甲=红，飞行=蓝） |
| `WBP_HUD` | Widget蓝图（父类TDHUDWidget） | 3个TextBlock绑定：TextLives/TextCost/TextExp |
| `BP_TDGameMode` | 蓝图（父类ATDGameMode） | BeginPlay: Create WBP_HUD → Set HUDWidget → Add to Viewport → UpdateDisplay |

#### 双货币体系

| 货币 | 用途 | 获取方式 |
|------|------|----------|
| 费用 (Cost) | 部署防御塔 | 初始0，每秒自动+1.0，上限99 |
| 作战记录 (Experience/EXP) | 升级防御塔(3档) | 击杀敌人掉落 |

#### 关键改动记录

| 日期 | 改动 |
|------|------|
| 6/1 | Zoom平滑插值：FInterpTo + ZoomStep步进钳制 |
| 6/1 | 伤害系统重做：EDamageType枚举，PhysicalArmor(物防) + MagicResistance(法抗%)，ApplyDamage() |
| 6/1 | HUD事件驱动：移除NativeTick，改为GameMode数据变化时调用UpdateDisplay() |
| 6/2 | Cost初始=0，MaxCost=99，恢复速率1.0/秒 |

#### 功能验证

- ✅ Spline路径绘制（Alt+拖拽控制点增删节点）
- ✅ 敌人沿Spline自动移动（面朝路径方向旋转）
- ✅ 到达Spline终点扣生命(LifeDamage) + 销毁 + GameOver判定
- ✅ 3种敌人差异（步/甲/飞：80/300/120血，400/150/250速，0/10/0+0/0/0防）
- ✅ 双防减伤：物理=攻击-防御(≥1)，法术=攻击×(1-抗性%)(≥1)
- ✅ 击杀敌人掉落经验（Die→GM->AddExperience）
- ✅ Cost初始0，每秒+1.0，上限99 (Timer驱动)
- ✅ Zoom平滑过渡（FInterpTo + 步进钳制）
- ✅ HUD事件驱动刷新（数据变化才更新UI，不Tick轮询）
- ✅ 全局Debug日志（EnemyLeaked/EXP+/GAME OVER/Cost变化）
- ✅ Source全部C++文件中文注释 (覆盖率>20%)
- ✅ 所有已提交保存

---

### 第3周 (6/2-) 防御塔系统

#### C++ 类

| 文件 | 说明 |
|------|------|
| `Source/ArkNightsDefence/Public/TDBaseTower.h` | 塔基类：继承AActor，TowerMesh/AttackRange+RangeSphere/AttackInterval/DamageType(EDamageType)/CostToDeploy，CurrentTarget/TowerLevel(1~3)，FindTarget()/Fire()/LevelUp() |
| `Source/ArkNightsDefence/Private/TDBaseTower.cpp` | Tick搜索+旋转朝向目标，SetTimer定时Fire→ApplyDamage，LevelUp消耗经验+更新攻击/间隔/范围+重启Timer |
| `Source/ArkNightsDefence/Public/TDGameMode.h` | 新增 SpendExperience(int32) 消耗经验接口 |
| `Source/ArkNightsDefence/Private/TDGameMode.cpp` | SpendExperience: 经验≥Amount则减经验+刷新HUD，否则打印红色提示 |

#### 蓝图资产

| 蓝图 | 类型 | 说明 |
|------|------|------|
| (待创建) `BP_Guard` | 蓝图类（父类TDBaseTower） | 近卫塔：单体物理高伤 |
| (待创建) `BP_Sniper` | 蓝图类（父类TDBaseTower） | 狙击塔：远程快速攻击 |
| (待创建) `BP_Caster` | 蓝图类（父类TDBaseTower） | 术士塔：范围法术AOE |

#### TDBaseTower 基础属性 (ClassDefaults)

| 参数 | 默认值 | 说明 |
|------|:---:|------|
| AttackDamage | 30.0 | 攻击力 |
| AttackRange | 300.0 | 攻击范围 |
| AttackInterval | 1.0s | 攻击间隔 |
| DamageType | Physical | 伤害类型 |
| CostToDeploy | 10.0 | 部署费用 |
| TowerLevel | 1 | 当前等级 |
| UpgradeCost_Lv2 | 50 | 升2级所需经验 |
| UpgradeCost_Lv3 | 100 | 升3级所需经验 |
| DamagePerLevel | +15 | 每级攻击力增量 |
| IntervalReducePerLevel | -0.15s | 每级攻速缩减 |
| RangePerLevel | +50 | 每级范围增量 |

#### 功能验证

- ✅ TDBaseTower 搜索最近敌人 (FindTarget, TActorIterator)
- ✅ 旋转炮塔朝向目标 (RInterpTo水平旋转)
- ✅ 定时攻击 (SetTimer, Fire→ApplyDamage)
- ✅ 3档升级 (LevelUp, 消耗SpendExperience)
- ✅ SpendExperience 接口 (GameMode新增)
- ⬜ 部署系统 (点击地面 → SpendCost → SpawnActor)
- ⬜ 弹丸系统 (TDProjectile)
- ⬜ 波次系统 (TDWaveManager)
