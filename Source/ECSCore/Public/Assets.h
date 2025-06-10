// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"

inline constexpr TCHAR AssetsFolder[] = TEXT("Assets");
inline constexpr TCHAR JsonExtension[] = TEXT(".json");
inline constexpr TCHAR FlecsExtension[] = TEXT(".flecs");
inline constexpr TCHAR TextExtension[] = TEXT(".txt");

namespace Assets {
	template<typename... Args>
	inline FString GetSavePath(const FString& extension, Args... args) {
		return FPaths::Combine(FPaths::ProjectSavedDir(), AssetsFolder, args...) + extension;
	}

	template<typename... Args>
	inline FString GetAssetPath(const FString& extension, Args&&... args) {
		FString localPath = FPaths::Combine(AssetsFolder, args...) + extension;
		FString savePath = FPaths::Combine(FPaths::ProjectSavedDir(), localPath);
		return FPaths::FileExists(savePath)
			? savePath
			: FPaths::Combine(FPaths::ProjectContentDir(), localPath);
	}

	template<typename... Args>
	inline std::vector<FString> GetFolders(const FString& path) {
		std::vector<FString> folders;
		IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

		platformFile.IterateDirectoryStat(*path, [&folders](const TCHAR* child, const FFileStatData& stat) -> bool {
			if (stat.bIsDirectory)
				folders.push_back(*FPaths::GetCleanFilename(child));
			return true;
			});

		return folders;
	}

	template<typename... Args>
	inline bool SaveJsonAsset(const FString& content, Args... args) {
		FString path = GetSavePath(JsonExtension, args...);
		IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString folderPath = FPaths::GetPath(path);
		while (!folderPath.IsEmpty()) {
			if (!platformFile.DirectoryExists(*folderPath))
				platformFile.CreateDirectory(*folderPath);
			folderPath = FPaths::GetPath(folderPath);
		}

		return FFileHelper::SaveStringToFile(content, *path);
	}

	template<typename... Args>
	inline char* LoadTextAsset(const FString& extension, Args... args) {
		FString content;
		if (!FFileHelper::LoadFileToString(content, *GetAssetPath(extension, args...)))
			return _strdup("");
		return _strdup(TCHAR_TO_UTF8(*content));
	}

	template<typename... Args>
	inline char* LoadJsonAsset(Args... args) {
		return LoadTextAsset(JsonExtension, args...);
	}

	template<typename... Args>
	inline char* LoadFlecsAsset(Args... args) {
		return LoadTextAsset(FlecsExtension, args...);
	}
};