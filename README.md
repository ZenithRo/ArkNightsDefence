# TowerDefence--一个仿ArkNights的塔防小游戏--基于UE5的C++大作业项目

### 5/26 项目立项

### 第1周 (5/27-5/29) 基础框架搭建

#### C++ 类

| 文件 | 说明 |
|------|------|
| `Source/ArkNightsDefence/Public/TDTopDownPawn.h` | Pawn头文件：继承APawn，声明SpringArm、Camera组件，MoveAction/ZoomAction输入引用，MoveSpeed/MinZoom/MaxZoom/ZoomStep可配置参数，Move()/Zoom()输入响应函数 |
| `Source/ArkNightsDefence/Private/TDTopDownPawn.cpp` | Pawn实现：构造函数创建USpringArmComponent+UCameraComponent（俯角60°、臂长2000）；Move()基于摄像机方向做WASD水平平移；Zoom()通过调整SpringArm臂长实现滚轮缩放 |
| `Source/ArkNightsDefence/Public/TDPlayerController.h` | PlayerController头文件：继承APlayerController，声明DefaultMappingContext、ClickAction，重写SetupInputComponent()，OnClick()鼠标点击响应 |
| `Source/ArkNightsDefence/Private/TDPlayerController.cpp` | PlayerController实现：BeginPlay显示鼠标光标并注册EnhancedInput；SetupInputComponent绑定ClickAction；OnClick()使用GetHitResultUnderCursor射线检测点击位置，绘制Debug小球+打印坐标 |
| `Source/ArkNightsDefence/ArkNightsDefence.Build.cs` | 构建配置：引入Core、CoreUObject、Engine、InputCore、EnhancedInput、UMG模块 |
| `Source/ArkNightsDefence/ArkNightsDefence.h` / `.cpp` | 模块入口文件 |
| `Source/ArkNightsDefence.Target.cs` / `ArkNightsDefenceEditor.Target.cs` | 编译目标配置 |

#### 蓝图资产（在编辑器中创建）

| 蓝图 | 类型 | 说明 |
|------|------|------|
| `BP_TDTopDownPawn` | 蓝图类（父类TDTopDownPawn） | 配置MoveAction→IA_Move、ZoomAction→IA_Zoom |
| `BP_TDPlayerController` | 蓝图类（父类TDPlayerController） | 配置DefaultMappingContext→IMC_TDGameplay、ClickAction→IA_Click |
| `BP_TDGameMode` | 蓝图类（父类GameModeBase） | 设置DefaultPawnClass和PlayerControllerClass |
| `IMC_TDGameplay` | Input Mapping Context | WASD→IA_Move、MouseWheel→IA_Zoom、LeftMouse→IA_Click |
| `IA_Move` | Input Action（Axis2D） | WASD四向移动输入 |
| `IA_Zoom` | Input Action（Axis1D） | 滚轮缩放输入 |
| `IA_Click` | Input Action（Digital/Bool） | 鼠标左键点击输入 |

#### 配置文件

| 文件 | 说明 |
|------|------|
| `Config/DefaultInput.ini` | 使用EnhancedInput系统，配置EnhancedPlayerInput和EnhancedInputComponent |
| `.gitignore` | 忽略.vs、Binaries、Intermediate、DerivedDataCache、Saved等生成文件 |

#### 功能验证

- ✅ 俯视角摄像机（SpringArm俯角60°）
- ✅ WASD四向平移（基于摄像机方向，帧率无关）
- ✅ 滚轮缩放（范围400~3000，步长200）
- ✅ 鼠标左键点击地面 → 射线检测 + Debug红球 + 屏幕坐标打印
- ✅ 内容已push到GitHub仓库
