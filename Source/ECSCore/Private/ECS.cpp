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
}