// Fill out your copyright notice in the Description page of Project Settings.

#include "Assets.h"

char* Assets::LoadJsonAsset(const FString& Key) {
	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *GetAssetPath(Key + JsonExtension)))
		return _strdup("");
	return _strdup(TCHAR_TO_UTF8(*FileContent));
}

bool Assets::SaveJsonAsset(const FString& Key, const FString& Content) {
	FString Path = GetSavePath(Key);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString Folder = FPaths::GetPath(Path);

	if (!PlatformFile.DirectoryExists(*Folder)) {
		PlatformFile.CreateDirectoryTree(*Folder);
	}

	return FFileHelper::SaveStringToFile(Content, *Path);
}

FString Assets::GetAssetPath(const FString& Filename) {
	FString LocalPath = FPaths::Combine(FPaths::ProjectSavedDir(), Root, Filename);
	return FPaths::FileExists(LocalPath)
		? LocalPath
		: FPaths::Combine(FPaths::ProjectContentDir(), Root, Filename);
}

FString Assets::GetSavePath(const FString& Filename) {
	return FPaths::Combine(FPaths::ProjectSavedDir(), Root, Filename);
}
