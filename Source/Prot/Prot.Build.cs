// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class Prot : ModuleRules
{
	public Prot(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "GameplayTasks",
			"RMPP"
		});
		PublicIncludePaths.AddRange(new string[]
		{
			"RMPP/Public", "RMPP/Classes"
		});
		
		
        PrivateDependencyModuleNames.AddRange(new string[]
        {
	        "Slate", "SlateCore"
        });
    }
}
