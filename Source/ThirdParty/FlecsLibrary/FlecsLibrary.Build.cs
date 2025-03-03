// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class FlecsLibrary : ModuleRules
{
	public FlecsLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		bUseUnity = false;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"JsonUtilities"
		});

		if (Target.LinkType == TargetLinkType.Monolithic)
		{
			PublicDefinitions.Add("flecs_STATIC");
		}
		else
		{
			PrivateDefinitions.Add("flecs_EXPORTS");
		}

		PublicDefinitions.Add("FLECS_CPP_NO_AUTO_REGISTRATION = 1");

		if (Target.Configuration < UnrealTargetConfiguration.Test)
		{
			PublicDefinitions.Add("FLECS_SOFT_ASSERT = 1");
		}

		if (Target.Configuration < UnrealTargetConfiguration.Development)
		{
			// A cheesy way to turn off NDEBUG for Flecs
			PublicDefinitions.Add("DUMMYDEFINE\n#ifdef NDEBUG\n#undef NDEBUG\n#endif");
			PublicDefinitions.Add("FLECS_DEBUG = 1");
			PublicDefinitions.Add("FLECS_SANITIZE = 1");
		}
	}
}
