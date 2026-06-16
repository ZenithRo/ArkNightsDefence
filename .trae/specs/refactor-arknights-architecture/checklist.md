# 验收清单

## 目录结构
- [x] `Public/Core/` 存在且包含核心类（GameMode, PlayerController, TopDownPawn）
- [x] `Public/Grid/` 存在且包含网格相关类（GridManager, GridDataActor）
- [x] `Public/Deployment/` 存在且包含部署相关类（DeploymentPreviewActor, DeploymentManager）
- [x] `Public/Enemy/` 存在且包含敌人相关类（EnemyBase, WaveManager）
- [x] `Public/Tower/` 存在且包含塔相关类（BaseTower, TargetSelector）
- [x] `Public/Ability/` 存在且包含技能相关存根
- [x] `Public/UI/` 存在且包含 UI 相关类（HUDWidget, HealthBarWidget）
- [x] `Private/` 下对应子目录结构完整

## 文件移动完整性
- [x] 所有原有 `.h` 文件已从 `Public/` 根目录移出，无残留
- [x] 所有原有 `.cpp` 文件已从 `Private/` 根目录移出，无残留
- [x] `#include` 路径使用子目录前缀（如 `#include "Grid/TDGridManager.h"`）

## 构建配置
- [x] `.Build.cs` 确认无需修改（UE5 自动递归扫描子目录）
- [x] 模块编译不报 `C1083`（无法打开包含文件）

## 存根模块
- [x] `TDTargetSelector.h` 存在，定义 `ETargetPriority` 和选择器类
- [x] `TDDeploymentManager.h` 存在，定义费用/部署接口
- [x] `TDWaveManager.h` 存在，定义波次配置和生成接口
- [x] `TDSkillData.h` 存在，定义技能数据结构

## 数据结构
- [x] `ETileType` 枚举已定义（GROUND, HIGHLAND, BLOCKED, START, END）
- [x] `FGridCellData` 已增加 `ETileType TileType` 字段
- [x] `EDamageType` 已与参考文档对齐（Physical, Magic）
- [x] `FDamageInfo` 结构体已定义

## 编译验证
- [x] 项目编译无报错（`Result: Succeeded`）
- [x] git 已同步所有变更
