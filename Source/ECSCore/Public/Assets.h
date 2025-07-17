// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"

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

	inline TArray<FString> GetFolders(const FString& path) {
		TArray<FString> folders;
		IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

		platformFile.IterateDirectoryStat(*path, [&folders](const TCHAR* child, const FFileStatData& stat) -> bool {
			if (stat.bIsDirectory)
				folders.Add(*FPaths::GetCleanFilename(child));
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

	inline char* LoadTextFile(const FString& path) {
		FString content;
		if (!FFileHelper::LoadFileToString(content, *path))
			return _strdup("");
		return _strdup(TCHAR_TO_UTF8(*content));
	}

	ECSCORE_API inline TArray<FString> SelectFiles(const FString& dialogTitle = TEXT("Select Files"), const FString& defaultPath = FPaths::ProjectDir(), const FString& fileTypes = TEXT("All Files (*.*)|*.*"))
	{
		TArray<FString> selectedFiles;

		IDesktopPlatform* desktopPlatform = FDesktopPlatformModule::Get();
		if (desktopPlatform) {
			void* parentWindowHandle = nullptr;

#if WITH_EDITOR
			const TSharedPtr<SWindow> parentWindow = FSlateApplication::IsInitialized() ? FSlateApplication::Get().FindBestParentWindowForDialogs(nullptr) : nullptr;
			if (parentWindow.IsValid()) {
				parentWindowHandle = parentWindow->GetNativeWindow()->GetOSWindowHandle();
			}
#endif

			desktopPlatform->OpenFileDialog(
				parentWindowHandle,
				dialogTitle,
				defaultPath,
				TEXT(""),
				fileTypes,
				EFileDialogFlags::Multiple,
				selectedFiles
			);
		}

		return selectedFiles;
	}
};