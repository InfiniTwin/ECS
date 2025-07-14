// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"
#include "Assets.h"

namespace ECS {
	TMap<FString, FString> Tokens;

	void RunScripts(flecs::world& world, const FString& folder, const TArray<FString>& files) {
		for (const FString& file : files) 
			RunScript(world, folder, file);
	}

	void RunCode(flecs::world& world, const FString& name, const FString& code) {
		FString scopedCode = SetScopes(code);
		if (ecs_script_run(world, TCHAR_TO_ANSI(*name), TCHAR_TO_UTF8(*scopedCode)))
			UE_LOG(LogTemp, Error, TEXT(">>> Error Running Flecs Script: %s"), *name);
	}
}