// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MyTournament : ModuleRules
{
	public MyTournament(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Niagara",
            "NavigationSystem"
        });

        PublicIncludePaths.AddRange(new string[]
        {
            "MyTournament"
        });
    }
}
