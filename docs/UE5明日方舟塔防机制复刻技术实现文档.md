# UE5 明日方舟风格塔防机制：已实现技术文档

本文只记录仓库当前 C++ 与蓝图资源中能够确认的实现，不把设计草案或未来计划写成已完成内容。

## 1. 系统分层

项目采用 C++ 基础类 + 蓝图资源配置的混合模式：GameMode 管理全局状态，PlayerController 负责玩家输入与部署交互，Pawn 负责摄像机，GridManager 负责网格数据，Enemy/WaveManager 负责敌人与波次，BaseTower 负责塔的攻击与升级，UMG 负责 HUD 与手牌显示。

## 2. 俯视角与输入

`ATDTopDownPawn` 创建 SpringArm 和 Camera。SpringArm 默认长度为 2000，俯角由 `CameraPitch` 配置；WASD 输入沿摄像机水平前方向和右方向移动 Pawn；滚轮通过 `TargetZoomDistance` 和 `FInterpTo` 平滑缩放。`ATDPlayerController` 注册 Enhanced Input Mapping Context，并绑定鼠标点击、P 暂停和 Space 倍速切换。

## 3. 网格与部署

`UTDGridManager` 使用一维数组保存二维格子，提供 `WorldToGrid`、`GridToWorld`、`CanDeployAtWithPlacement`、`TryOccupy` 和 `Free`。地面、高台、阻挡、洞穴等类型由 `ETileType` 表示，塔的地面/高台限制由 `ETowerPlacement` 表示。部署流程、GhostActor 和费用校验详见《部署系统设计文档》。

## 4. 敌人路径与波次

`ATDEnemy` 从 `PathActor` 获取 SplineComponent，在 Tick 中按 `MoveSpeed` 推进 `DistanceAlongSpline`，并根据路径方向更新位置和动画。敌人拥有地面/飞行类型、生命、速度、物理护甲、法术抗性、击杀经验和终点伤害。受到攻击时分别计算物理伤害减护甲、法术伤害乘以抗性减免，并支持穿透百分比。到达终点会广播事件，由 GameMode 扣除玩家生命。

`ATDWaveManager` 读取 `FTDWaveTableRow` DataTable。每一行包含波次延迟和多个生成条目，每个条目指定敌人类、数量、生成间隔和路径索引。管理器按照配置生成敌人，统计击杀数/总数/当前波次，并通过委托和 GameMode Tick 驱动 HUD 更新。

## 5. 防御塔与战斗

`ATDBaseTower` 是所有防御塔蓝图的 C++ 基类，包含网格坐标、静态网格、攻击范围球、Spine 动画组件、生命和防御属性。攻击支持：

- 单体或群体攻击；
- 地面、飞行或两者兼容的目标过滤；
- 圆形距离范围或配置的格子偏移范围；
- 物理/法术伤害及对应防御穿透；
- 对敌人施加减速；
- 医疗塔按范围内友方塔的生命百分比选择目标并治疗。

普通攻击会从有效敌人中选择 `DistanceAlongSpline` 最大者作为当前目标。`UTDGTargetSelector` 还提供最近、最远、最低/最高 HP 等排序模式；当前代码对防御最低、最高重量、最先阻挡等枚举值尚未实现专门排序。

## 6. 塔等级与手牌

`FTowerLevelStats` 保存每级生命、攻击、攻击间隔、部署费用、物理护甲和法抗。塔的等级上限为 3，升级分别消耗 `UpgradeCost_Lv2` 与 `UpgradeCost_Lv3`。手牌保存每个塔类的当前等级，升级成功后刷新 `UTDHandPanel`；部署时将手牌等级传给新塔，避免重复支付升级费用。

## 7. HUD、血条和音频

`UTDHUDWidget::UpdateDisplay` 从 GameMode 读取生命、费用、经验和波次状态，使用事件/状态变化驱动刷新而非 HUD Tick 轮询。`UTDHealthBarWidget` 使用 Slate ProgressBar 绘制并更新颜色。`UTDMusicManager` 作为子系统封装音乐播放、停止、淡出和播放进度查询。

## 8. 未纳入已完成需求的内容

技能/GAS、天赋、Boss、完整的胜负结算界面、复杂敌人 Buff、多人同步和对象池没有在当前 C++ 运行流程中形成可验证闭环，课程设计报告中将它们放入个人总结的“项目可完善的内容”，不列入需求分析的已实现功能。

