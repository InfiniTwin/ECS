// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FlecsLibrary : ModuleRules
{
	public FlecsLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core" });

		if (Target.Platform != UnrealTargetPlatform.Win64) 
			AppendStringToPublicDefinition("flecs_EXPORTS", "0");

		PrivateDefinitions.Add("flecs_EXPORTS");
	}
}
