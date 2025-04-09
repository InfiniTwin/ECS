// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

inline constexpr TCHAR AssetsFolder[] = TEXT("Assets");
inline constexpr TCHAR JsonExtension[] = TEXT(".json");

struct ECSCORE_API Assets {
public:
	static char* LoadJsonAsset(const FString& Key);
	static bool SaveJsonAsset(const FString& Key, const FString& Content);
	static FString GetAssetPath(const FString& Filename);
	static FString GetSavePath(const FString& Filename);
};
