// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"

namespace ECS {
	void RunScript(
		flecs::world& world,
		const FString& path,
		const FString& scope,
		const FString& parent,
		LoadMode mode,
		bool invert)
	{
		using namespace Assets;
		char* data = LoadFlecsAsset(path);
		FString scopedData = SetScope(data, scope);
		free(data);
		if (ecs_script_run(world, TCHAR_TO_ANSI(*path), TCHAR_TO_UTF8(*scopedData)))
			UE_LOG(LogTemp, Error, TEXT(">>> Error Running Flecs Script: %s"), *path);
	}
}