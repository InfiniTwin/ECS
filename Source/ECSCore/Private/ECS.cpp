// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"

namespace ECS {
	TMap<FString, FString> Tokens;

	void RunScript(flecs::world& world, const FString& path, const FString& file, const FString& target) {
		FString filePath = Assets::GetAssetPath(FlecsExtension, path, file);
		FString code;
		if (FFileHelper::LoadFileToString(code, *filePath))
			RunCode(world, filePath, *code, target);
	}

	void RunScripts(flecs::world& world, const FString& path, const TArray<FString>& files, const FString& target) {
		for (const FString& file : files)
			RunScript(world, path, file, target);
	}

	void RunCode(flecs::world& world, const FString& name, const FString& code, const FString& target) {
		auto codeValue = code;
		codeValue.ReplaceInline(TARGET, *target);
		codeValue = SetScopes(codeValue);
		if (ecs_script_run(world, TCHAR_TO_ANSI(*name), TCHAR_TO_UTF8(*codeValue)))
			UE_LOG(LogTemp, Error, TEXT(">>> Error Running Flecs Script: %s"), *name);
	}
}