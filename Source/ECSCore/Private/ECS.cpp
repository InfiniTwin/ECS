// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"

namespace ECS {
	TMap<FString, FString> Scopes;

	void RunScript(flecs::world& world, const FString& path) {
		using namespace Assets;
		char* data = LoadFlecsAsset(path);
		FString code = UTF8_TO_TCHAR(data);
		free(data);
		RunScript(world, path, code);
	}

	void RegisterOpaqueTypes(flecs::world& world) {
		// FString <=> flecs::String
		world.component<FString>()
			.opaque(flecs::String)
			.serialize([](const flecs::serializer* s, const FString* data) {
			const char* str = TCHAR_TO_UTF8(**data);
			return s->value(flecs::String, &str);
				})
			.assign_string([](FString* data, const char* value) {
			*data = UTF8_TO_TCHAR(value);
				});

		// FText <=> flecs::String
		world.component<FText>()
			.opaque(flecs::String)
			.serialize([](const flecs::serializer* s, const FText* data) {
			FString temp = data->ToString();
			const char* str = TCHAR_TO_UTF8(*temp);
			return s->value(flecs::String, &str);
				})
			.assign_string([](FText* data, const char* value) {
			*data = FText::FromString(UTF8_TO_TCHAR(value));
				});
	}
}