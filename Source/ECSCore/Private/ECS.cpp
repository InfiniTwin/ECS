// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"

namespace ECS {
	TMap<FString, FString> Tokens;

	bool LoadScriptFile(const FString& path, const FString& file, FString& outFilePath, FString& outCode) {
		outFilePath = Assets::GetAssetPath(FlecsExtension, path, file);
		return FFileHelper::LoadFileToString(outCode, *outFilePath);
	}

	FString FormatCode(const FString& data, const FString& target) {
		FString result = data;
		result.ReplaceInline(TARGET, *target);
		for (const auto& Pair : Tokens)
			result = result.Replace(*Pair.Key, *Pair.Value, ESearchCase::CaseSensitive);
		return result;
	}


	void RunScript(flecs::world& world, const FString& path, const FString& file, const FString& target) {
		FString filePath, code;
		if (LoadScriptFile(path, file, filePath, code))
			RunCode(world, filePath, *code, target);
	}

	void RunScripts(flecs::world& world, const FString& path, const TArray<FString>& files, const FString& target) {
		for (const FString& file : files)
			RunScript(world, path, file, target);
	}

	void RunCode(flecs::world& world, const FString& name, const FString& code, const FString& target) {
		if (ecs_script_run(world, TCHAR_TO_ANSI(*name), TCHAR_TO_UTF8(*FormatCode(code, target))))
			UE_LOG(LogTemp, Warning, TEXT(">>> Failed to Run Flecs Script: %s"), *name);
	}

	TArray<FString> GetEntityPaths(const FString& scriptText) {
		TArray<FString> result;
		TArray<FString> lines;
		scriptText.ParseIntoArrayLines(lines);

		const FRegexPattern pattern(TEXT(R"(^(\:\:[A-Za-z0-9_]+(?:::[A-Za-z0-9_]+)*\.[A-Za-z0-9_]+)(\s*:\s*[A-Za-z0-9_,\s]+)?\s*\{)"));

		for (const FString& line : lines) {
			FRegexMatcher matcher(pattern, line);
			if (matcher.FindNext()) {
				FString path = matcher.GetCaptureGroup(1);
				result.Add(ECS::FullPath(path));
			}
		}

		return result;
	}

	void ClearScript(flecs::world& world, const FString& path, const FString& file, const FString& target) {
		FString filePath, code;
		if (!LoadScriptFile(path, file, filePath, code))
			return;

		FString formated = *FormatCode(code, target);

		TArray<FString> paths = GetEntityPaths(formated);
		for (const FString& fullPath : paths)
			if (flecs::entity entity = world.lookup(TCHAR_TO_UTF8(*fullPath)); entity.is_alive())
				entity.destruct();
	}
}