# 项目架构重构梳理 Spec

## Why

当前项目代码缺乏子系统化的目录组织，与参考「UE5明日方舟塔防机制复刻技术实现文档」的结构相比存在较大差距。本次重构旨在：
- 将代码按照功能领域划分为子目录（格子系统、部署系统、敌人系统等）
- 补齐参考文档中已定义但本项目缺失的基础结构
- 保持现有功能完整的**同时**为新系统预留扩展点
- **不进行大规模重命名**：保留 TD 前缀命名风格，避免破坏蓝图引用

## 参考文档 vs 当前项目 对照表

| 参考文档章节 | 当前项目状态 | 策略 |
|-------------|-------------|------|
| **1. 关卡格子系统** | 已实现 `TDGridManager`（矩形网格），但参考使用六边形轴向坐标 + 地形掩码 | **重构**：保留矩形网格，添加 ETileType 地形类型枚举和双层地形（地面/高台）概念 |
| **2. 部署系统** | 已实现基本部署（点击→扣费→生成塔），但缺少手牌系统、方向选择、再部署冷却 | **部分重构**：结构化现有代码，方向选择和手牌系统留空 |
| **3. 敌人路径与 AI 系统** | 已实现 `TDEnemy`（Spline 跟随+阻挡+近战攻击），缺少波次管理器 | **重构**：提取敌人基类接口，添加波次管理器存根 |
| **4. 攻击系统** | 已有基础攻击（定时器+伤害），缺少范围矩阵、弹道、GAS | **部分重构**：添加攻击范围数据结构，GAS 留空 |
| **5. 技能系统** | **未实现** | **留空**：创建目录和基础文件 |
| **6. 目标选择优先级系统** | **未实现** | **留空**：创建基础数据结构 |

## 目录结构映射

```
Source/ArkNightsDefence/
├── Public/
│   ├── Core/                    # 核心系统
│   │   ├── TDGameMode.h         (已有)
│   │   ├── TDPlayerController.h (已有)
│   │   └── TDGridManager.h      (已有 → 移入)
│   ├── Grid/                    # 格子系统 (参考 §1)
│   │   ├── TDGridManager.h      (从Core移入)
│   │   └── TDGridDataActor.h    (编辑器工具)
│   ├── Deployment/              # 部署系统 (参考 §2)
│   │   ├── TDDeploymentPreviewActor.h (已有)
│   │   └── TDDeploymentManager.h     (新建·存根)
│   ├── Enemy/                   # 敌人系统 (参考 §3)
│   │   ├── TDEnemy.h            (已有)
│   │   └── TDWaveManager.h      (新建·存根)
│   ├── Tower/                   # 塔/干员系统 (参考 §4 攻击)
│   │   ├── TDBaseTower.h        (已有)
│   │   └── TDTargetSelector.h   (新建·存根)
│   ├── Ability/                 # 技能系统 (参考 §5)
│   │   └── TDSkillData.h        (新建·存根)
│   └── UI/                      # UI 系统
│       ├── TDHUDWidget.h        (已有)
│       └── TDHealthBarWidget.h  (已有)
└── Private/                     # 对应 .cpp 文件 (目录结构一致)
```

## 变更类型说明

- **REFACTOR**：将现有 .h/.cpp 文件移动到对应子目录，修改 `#include` 路径和 `.Build.cs`
- **MODIFY**：修改现有类的内部实现（如添加新字段、接口）
- **ADD**：创建新的类/文件（存根）
- **NO_CHANGE**：保留现有状态（如 TD-prefix 命名风格）

## 具体变更计划

### Phase 1: 目录结构调整（REFACTOR）

移动现有文件到子目录，更新 `#include` 和 `.Build.cs`。

| 文件 | 当前路径 | 目标路径 |
|------|---------|---------|
| TDGameMode.h/cpp | Public/Private | Public/Core, Private/Core |
| TDGridManager.h/cpp | Public/Private | Public/Grid, Private/Grid |
| TDDeploymentPreviewActor.h/cpp | Public/Private | Public/Deployment, Private/Deployment |
| TDEnemy.h/cpp | Public/Private | Public/Enemy, Private/Enemy |
| TDBaseTower.h/cpp | Public/Private | Public/Tower, Private/Tower |
| TDHUDWidget.h/cpp | Public/Private | Public/UI, Private/UI |
| TDHealthBarWidget.h/cpp | Public/Private | Public/UI, Private/UI |
| TDPlayerController.h/cpp | Public/Private | Public/Core, Private/Core |
| TDTopDownPawn.h/cpp | Public/Private | Public/Core, Private/Core |
| TDGridDataActor.h/cpp | Public/Private | Public/Grid, Private/Grid |

### Phase 2: 数据结构补齐（MODIFY）

参考文档中的关键但缺失的数据结构：

1. **格子系统**：添加 `ETileType` 枚举、`FGridCell` 结构体丰富格子系统
2. **塔系统**：添加 `EDamageType` 完善伤害类型枚举
3. **敌人系统**：添加 `FDamageInfo` 伤害信息结构体

### Phase 3: 存根模块创建（ADD）

创建以下文件作为占位（只有基础声明，不含实现）：

1. `Tower/TDTargetSelector.h` — 目标选择优先级系统（参考 §6）
2. `Deployment/TDDeploymentManager.h` — 部署管理器（参考 §2）
3. `Enemy/TDWaveManager.h` — 波次管理器（参考 §3）
4. `Ability/TDSkillData.h` — 技能数据结构（参考 §5）

### Phase 4: 更新构建文件和提交

更新 `.Build.cs` 确保新目录被识别，然后 git commit。

## 不做的事（明确不在此次范围内）

- ❌ 重命名 TD 前缀类名 → 现有蓝图绑定稳定
- ❌ 实现完整的 GAS 技能系统 → 留空后期再做
- ❌ 修改现有功能逻辑（攻击、部署等）→ 仅结构化
- ❌ 六边形网格系统 → 保留矩形网格
