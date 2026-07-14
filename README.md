# ArkNightsDefence：UE5 C++ 俯视角塔防课程设计

## 项目简介

本项目使用 Unreal Engine 5 与 C++/蓝图混合开发，实现一个以格子为基础的俯视角塔防原型（可以认为是游戏《明日方舟》的UE5复刻版本）。玩家通过 WASD 平移视野、鼠标滚轮缩放镜头，从手牌选择防御塔并拖拽到合法格子；敌人按照关卡中的 Spline 路径移动，防御塔依据攻击范围和目标类型自动攻击，玩家通过费用部署，通过作战经验升级手牌。

项目重点是 UE5 Actor、GameMode、PlayerController、UObject 数据管理器、Enhanced Input、UMG、DataTable、Spine 动画插件之间的协作。

## 已实现功能

- 俯视角摄像机：WASD 平移，鼠标滚轮平滑缩放。
- 网格系统：网格坐标转换、地面/高台/阻挡/洞穴等格子类型、占用状态和部署合法性检查。
- 部署系统：手牌拖拽、预览模型、地面/高台部署类型限制、费用检查、部署后格子占用。
- 敌人系统：地面与飞行敌人、Spline 路径移动、血条、物理/法术伤害、护甲/法抗、减速、阻挡和到达终点扣生命。
- 波次系统：DataTable 配置波次、敌人类型/数量/间隔/路径，统计当前波次和击杀进度。
- 防御塔系统：单体/群体攻击、地面/飞行目标限制、圆形/矩阵攻击范围、医疗塔、减速和无视防御比例、Spine 动画状态、三级属性。
- 经济与 UI：费用按秒恢复，击杀获得经验，HUD 显示生命/费用/经验/波次，手牌支持等级显示和经验升级。
- 交互控制：P 键暂停，Space 键切换 1 倍/2 倍时间速率。
- 音频：音乐子系统支持播放、停止、淡出和播放进度查询。

## 主要目录

| 目录 | 内容 |
|---|---|
| `Source/ArkNightsDefence/Public/Core` | GameMode、PlayerController、俯视角 Pawn |
| `Source/ArkNightsDefence/Public/Grid` | 网格、格子数据资产、编辑器网格 Actor |
| `Source/ArkNightsDefence/Public/Deployment` | 部署管理相关 Actor |
| `Source/ArkNightsDefence/Public/Enemy` | 敌人、波次数据和波次管理器 |
| `Source/ArkNightsDefence/Public/Tower` | 防御塔、攻击范围、目标选择、等级属性 |
| `Source/ArkNightsDefence/Public/UI` | HUD、手牌、血条组件 |
| `Content/Maps` | 地图、关卡 GameMode、波次 DataTable |
| `Content/Core/Blueprints` | C++ 类的蓝图子类与场景资源 |

## 编译与运行

1. 使用 Unreal Engine 5 打开 `ArkNightsDefence.uproject`。
2. 生成/编译项目的 Development Editor 配置。
3. 打开 `Content/Maps/Levels` 中的关卡，通过编辑器 Play 运行。
4. 使用手牌按钮进入部署预览，按住并移动鼠标后松开完成部署；使用 P 暂停，使用 Space 切换速度。

## 已知未完成或待完善内容

以下内容不作为本次课程设计的已完成需求：`ATDDeploymentManager::StartCostRegen` 目前为空；技能/GAS 系统未接入实际运行流程；目标选择器中部分枚举值尚未实现专门排序逻辑；结束界面、完整胜负状态机和更丰富的特效音效仍可继续完善。

## 文档

- [部署系统设计文档](docs/部署系统设计文档.md)
- [手牌系统设计文档](docs/手牌系统设计文档.md)
- [已实现机制技术文档](docs/UE5明日方舟塔防机制复刻技术实现文档.md)

## 注意
此项目只作学习所用，其中的Spine动画、音乐、美术资源为 ‌上海鹰角网络科技有限公司 版权所有,禁止用于非法用途。
