// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"
#include "Assets.h"

namespace ECS {
	TMap<FString, FString> Tokens;

	void RunScript(flecs::world& world, const FString& path) {
		using namespace Assets;
		char* data = LoadFlecsAsset(path);
		FString code = UTF8_TO_TCHAR(data);
		free(data);
		RunScript(world, path, code);
	}

	void RunScript(flecs::world& world, const FString& name, const FString& code) {
		FString scopedCode = SetScopes(code);
		if (ecs_script_run(world, TCHAR_TO_ANSI(*name), TCHAR_TO_UTF8(*scopedCode)))
			UE_LOG(LogTemp, Error, TEXT(">>> Error Running Flecs Script: %s"), *name);
	}
}