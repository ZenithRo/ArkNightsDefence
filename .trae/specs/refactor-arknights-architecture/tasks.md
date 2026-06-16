# 任务列表

> 根目录：`Source/ArkNightsDefence/`

## 前置说明
- 所有移动和重定向操作完成后，必须更新 `.Build.cs` 中的 `PublicIncludePaths` / `PrivateIncludePaths` 或调整根命名空间，确保编译通过。
- 每次文件变更后同步 git commit。

## Task 1: 创建子目录结构
- [x] 在 `Public/` 下创建 `Core/` `Grid/` `Deployment/` `Enemy/` `Tower/` `Ability/` `UI/` 子目录
- [x] 在 `Private/` 下创建对应的子目录
- [x] 同步 git

## Task 2: 移动现有 .h 文件到子目录
- [x] TDGameMode.h → Public/Core/
- [x] TDPlayerController.h → Public/Core/
- [x] TDTopDownPawn.h → Public/Core/
- [x] TDGridManager.h → Public/Grid/
- [x] TDGridDataActor.h → Public/Grid/
- [x] TDDeploymentPreviewActor.h → Public/Deployment/
- [x] TDEnemy.h → Public/Enemy/
- [x] TDBaseTower.h → Public/Tower/
- [x] TDHUDWidget.h → Public/UI/
- [x] TDHealthBarWidget.h → Public/UI/
- [x] 同步 git

## Task 3: 移动现有 .cpp 文件到子目录
- [x] TDGameMode.cpp → Private/Core/
- [x] TDPlayerController.cpp → Private/Core/
- [x] TDTopDownPawn.cpp → Private/Core/
- [x] TDGridManager.cpp → Private/Grid/
- [x] TDGridDataActor.cpp → Private/Grid/
- [x] TDDeploymentPreviewActor.cpp → Private/Deployment/
- [x] TDEnemy.cpp → Private/Enemy/
- [x] TDBaseTower.cpp → Private/Tower/
- [x] TDHUDWidget.cpp → Private/UI/
- [x] TDHealthBarWidget.cpp → Private/UI/
- [x] 同步 git

## Task 4: 更新 include 路径
- [x] 修改所有 `.h` 和 `.cpp` 文件中的 `#include` 路径，使用子目录相对路径
- [x] 检查 `.Build.cs` 中的 `PublicIncludePaths` 和 `PrivateIncludePaths` 是否需要添加(UE5自动递归扫描,无需修改)
- [x] 同步 git

## Task 5: 更新 .Build.cs
- [x] 检查模块注册路径是否需要变更(UE5自动扫描子目录,无需修改)
- [x] 确认 `PublicDependencyModuleNames` 和 `PrivateDependencyModuleNames` 完整
- [x] 同步 git

## Task 6: 数据结构补齐
- [x] 在 Grid/ 添加 `ETileType` 枚举和 `FGridCell` 结构体字段增强
- [x] 在 Tower/ 确认 `EDamageType` (已对齐参考文档: Physical/Magic)
- [x] 添加 `FDamageInfo` 伤害信息结构体
- [x] 同步 git

## Task 7: 创建存根模块
- [x] 创建 `Tower/TDTargetSelector.h`（目标选择器·存根，参考 §6）
- [x] 创建 `Deployment/TDDeploymentManager.h`（部署管理器·存根，参考 §2）
- [x] 创建 `Enemy/TDWaveManager.h`（波次管理器·存根，参考 §3.2.3）
- [x] 创建 `Ability/TDSkillData.h`（技能数据·存根，参考 §5）
- [x] 创建对应的 .cpp 空实现
- [x] 同步 git

## Task 8: 编译验证
- [x] 运行编译 → 成功 (Result: Succeeded)
- [x] 修复编译错误(前向声明修复 + 自引用include修复 + 跨子目录include修复 + 存根.cpp实现)
- [x] 最终 git commit

# 任务依赖关系
- Task 1 → Task 2/3 (目录先建好)
- Task 2/3 → Task 4 (文件到位后才能改路径)
- Task 4 → Task 5 (include 路径确定后才能确认 Build.cs)
- Task 5 → Task 8 (必须先能编译)
- Task 6 与 Task 7 可并行
