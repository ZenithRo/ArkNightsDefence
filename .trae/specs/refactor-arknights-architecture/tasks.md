# 任务列表

> 根目录：`Source/ArkNightsDefence/`

## 前置说明
- 所有移动和重定向操作完成后，必须更新 `.Build.cs` 中的 `PublicIncludePaths` / `PrivateIncludePaths` 或调整根命名空间，确保编译通过。
- 每次文件变更后同步 git commit。

## Task 1: 创建子目录结构
- [ ] 在 `Public/` 下创建 `Core/` `Grid/` `Deployment/` `Enemy/` `Tower/` `Ability/` `UI/` 子目录
- [ ] 在 `Private/` 下创建对应的子目录
- [ ] 同步 git

## Task 2: 移动现有 .h 文件到子目录
- [ ] TDGameMode.h → Public/Core/
- [ ] TDPlayerController.h → Public/Core/
- [ ] TDTopDownPawn.h → Public/Core/
- [ ] TDGridManager.h → Public/Grid/
- [ ] TDGridDataActor.h → Public/Grid/
- [ ] TDDeploymentPreviewActor.h → Public/Deployment/
- [ ] TDEnemy.h → Public/Enemy/
- [ ] TDBaseTower.h → Public/Tower/
- [ ] TDHUDWidget.h → Public/UI/
- [ ] TDHealthBarWidget.h → Public/UI/
- [ ] 同步 git

## Task 3: 移动现有 .cpp 文件到子目录
- [ ] TDGameMode.cpp → Private/Core/
- [ ] TDPlayerController.cpp → Private/Core/
- [ ] TDTopDownPawn.cpp → Private/Core/
- [ ] TDGridManager.cpp → Private/Grid/
- [ ] TDGridDataActor.cpp → Private/Grid/
- [ ] TDDeploymentPreviewActor.cpp → Private/Deployment/
- [ ] TDEnemy.cpp → Private/Enemy/
- [ ] TDBaseTower.cpp → Private/Tower/
- [ ] TDHUDWidget.cpp → Private/UI/
- [ ] TDHealthBarWidget.cpp → Private/UI/
- [ ] 同步 git

## Task 4: 更新 include 路径
- [ ] 修改所有 `.h` 和 `.cpp` 文件中的 `#include` 路径，使用子目录相对路径
- [ ] 检查 `.Build.cs` 中的 `PublicIncludePaths` 和 `PrivateIncludePaths` 是否需要添加
- [ ] 同步 git

## Task 5: 更新 .Build.cs
- [ ] 检查模块注册路径是否需要变更
- [ ] 确认 `PublicDependencyModuleNames` 和 `PrivateDependencyModuleNames` 完整
- [ ] 同步 git

## Task 6: 数据结构补齐
- [ ] 在 Grid/ 添加 `ETileType` 枚举和 `FGridCell` 结构体（参考文档 §1.2）
- [ ] 在 Tower/ 完善伤害类型枚举 `EDamageType`(已有) → 确保与参考文档对齐
- [ ] 同步 git

## Task 7: 创建存根模块
- [ ] 创建 `Tower/TDTargetSelector.h`（目标选择器·存根，参考 §6）
- [ ] 创建 `Deployment/TDDeploymentManager.h`（部署管理器·存根，参考 §2）
- [ ] 创建 `Enemy/TDWaveManager.h`（波次管理器·存根，参考 §3.2.3）
- [ ] 创建 `Ability/TDSkillData.h`（技能数据·存根，参考 §5）
- [ ] 同步 git

## Task 8: 编译验证
- [ ] 运行 `$env:UBT_MAX_PARALLEL=2; & "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ArkNightsDefenceEditor Win64 Development -Project="..." -WaitMutex -FromMsBuild -architecture=x64`
- [ ] 修复编译错误
- [ ] 最终 git commit

# 任务依赖关系
- Task 1 → Task 2/3 (目录先建好)
- Task 2/3 → Task 4 (文件到位后才能改路径)
- Task 4 → Task 5 (include 路径确定后才能确认 Build.cs)
- Task 5 → Task 8 (必须先能编译)
- Task 6 与 Task 7 可并行
