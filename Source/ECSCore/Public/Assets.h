// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct ECSCORE_API Assets
{
public:
    static char* LoadJsonAsset(const FString& Key);
    static bool SaveJsonAsset(const FString& Key, const FString& Content);

private:
    static constexpr const TCHAR* Root = TEXT("Assets");
    static constexpr const TCHAR* JsonExtension = TEXT(".json");

    static FString GetAssetPath(const FString& Filename);
    static FString GetSavePath(const FString& Filename);
};
