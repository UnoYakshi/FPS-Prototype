// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Prot : ModuleRules
{
	public Prot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "GameplayTasks" });

        // Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
        // BEGIN STEAM INTEGRATION

        // Uncomment if you are using online features
        //PrivateDependencyModuleNames.Add("OnlineSubsystem");
        //PrivateDependencyModuleNames.Add("OnlineSubsystemNull");
        //if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux))
        //{
        //    DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        //}

        // END STEAM INTEGRATION
    }
}
