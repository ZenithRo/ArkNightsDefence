// ArkNightsDefence 模块构建配置: 声明依赖的引擎模块
// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ArkNightsDefence : ModuleRules
{
    public ArkNightsDefence(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 公共依赖模块: 在.h中引用的引擎模块
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",           // 核心类型和容器
            "CoreUObject",    // UObject系统和反射
            "Engine",         // 游戏框架基础
            "InputCore",      // 传统输入系统支持
            "EnhancedInput",  // Enhanced Input增强输入系统
            "UMG"             // UMG UI控件系统
        });

        // 私有依赖模块
        PrivateDependencyModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
