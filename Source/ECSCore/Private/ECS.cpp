// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"

namespace ECS {
	TMap<FString, FString> Scopes;

	const TMap<FString, FString>& EmptyTokens() {
		static const TMap<FString, FString> empty;
		return empty;
	}

	TMap<FString, FString> Tokens(const TArray<TPair<FString, FString>>& pairs) {
		TMap<FString, FString> map;
		for (const auto& pair : pairs)
			map.Add(pair.Key, pair.Value);
		return map;
	}

	FString FormatCode(const FString& code, const TMap<FString, FString>& tokens = EmptyTokens()) {
		FString result = code;
		for (const auto& pair : tokens)
			result.ReplaceInline(*pair.Key, *pair.Value, ESearchCase::CaseSensitive);
		for (const auto& pair : Scopes)
			result.ReplaceInline(*pair.Key, *pair.Value, ESearchCase::CaseSensitive);
		return result;
	}

	bool LoadScriptFile(const FString& path, const FString& file, FString& outFilePath, FString& outCode) {
		outFilePath = Assets::GetAssetPath(FlecsExtension, path, file);
		return FFileHelper::LoadFileToString(outCode, *outFilePath);
	}

	void RunScript(flecs::world& world, const FString& path, const FString& file, const TMap<FString, FString>& tokens) {
		FString filePath, code;
		if (LoadScriptFile(path, file, filePath, code))
			RunCode(world, filePath, *code, tokens);
	}

	void RunScripts(flecs::world& world, const FString& path, const TArray<FString>& files, const TMap<FString, FString>& tokens) {
		for (const FString& file : files)
			RunScript(world, path, file, tokens);
	}

	void RunCode(flecs::world& world, const FString& name, const FString& code, const TMap<FString, FString>& tokens) {
		FString formated = FormatCode(code, tokens);

		ecs_script_eval_result_t result = { 0 };

		if (ecs_script_run(world, TCHAR_TO_ANSI(*name), TCHAR_TO_UTF8(*formated), &result)) {
			UE_LOG(LogTemp, Error, TEXT(">>> Error: %s\nScript: %s"), UTF8_TO_TCHAR(result.error), *name);
			ecs_os_free(result.error);
		}

		UE_LOG(LogTemp, Warning, TEXT(">>> Success: %s\n%s"), *name, *formated);
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

	void ClearScript(flecs::world& world, const FString& path, const FString& file, const TMap<FString, FString>& tokens) {
		FString filePath, code;
		if (!LoadScriptFile(path, file, filePath, code))
			return;

		FString formated = *FormatCode(code, tokens);

		TArray<FString> paths = GetEntityPaths(formated);
		for (const FString& fullPath : paths)
			if (flecs::entity entity = world.lookup(TCHAR_TO_UTF8(*fullPath)); entity.is_alive())
				entity.destruct();
	}
}