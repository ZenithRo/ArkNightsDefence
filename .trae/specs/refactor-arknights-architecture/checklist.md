# 验收清单

## 目录结构
- [ ] `Public/Core/` 存在且包含核心类（GameMode, PlayerController, TopDownPawn）
- [ ] `Public/Grid/` 存在且包含网格相关类（GridManager, GridDataActor）
- [ ] `Public/Deployment/` 存在且包含部署相关类（DeploymentPreviewActor, DeploymentManager）
- [ ] `Public/Enemy/` 存在且包含敌人相关类（EnemyBase, WaveManager）
- [ ] `Public/Tower/` 存在且包含塔相关类（BaseTower, TargetSelector）
- [ ] `Public/Ability/` 存在且包含技能相关存根
- [ ] `Public/UI/` 存在且包含 UI 相关类（HUDWidget, HealthBarWidget）
- [ ] `Private/` 下对应子目录结构完整

## 文件移动完整性
- [ ] 所有原有 `.h` 文件已从 `Public/` 根目录移出，无残留
- [ ] 所有原有 `.cpp` 文件已从 `Private/` 根目录移出，无残留
- [ ] `#include` 路径使用子目录前缀（如 `#include "Grid/TDGridManager.h"`）

## 构建配置
- [ ] `.Build.cs` 中的 `PublicIncludePaths` 包含所有子目录
- [ ] 模块编译不报 `C1083`（无法打开包含文件）

## 存根模块
- [ ] `TDTargetSelector.h` 存在，定义 `ETargetPriority` 和 `UTargetSelector` 类
- [ ] `TDDeploymentManager.h` 存在，定义手牌/费用/部署接口
- [ ] `TDWaveManager.h` 存在，定义波次配置和生成接口
- [ ] `TDSkillData.h` 存在，定义技能数据结构

## 数据结构
- [ ] `ETileType` 枚举已定义（GROUND, HIGHLAND, BLOCKED 等）
- [ ] `FGridCell` 结构体已定义（Coordinate, TileType, bIsOccupied 等）
- [ ] `EDamageType` 已与参考文档对齐（Physical, Magical 等）

## 编译验证
- [ ] 项目编译无报错（`Result: Succeeded`）
- [ ] git 已同步所有变更
