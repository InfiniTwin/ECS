// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ECSCore : ModuleRules
{
	public ECSCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
				"Core",
				"FlecsLibrary",
				"Projects"
			});


		PrivateDependencyModuleNames.AddRange(new string[] {
				"CoreUObject",
				"Engine",
			});


		DynamicallyLoadedModuleNames.AddRange(new string[] {
			});
	}
}
