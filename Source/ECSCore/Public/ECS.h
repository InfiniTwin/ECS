// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"

namespace ECS {
#define COMPONENT(T) ([] { return #T; }())

	constexpr const char* Member(const char* str) {
		const char* lastColon = str;
		for (const char* ptr = str; *ptr != '\0'; ++ptr)
			if (*ptr == ':')
				lastColon = ptr + 1;
		return lastColon;
	}
#define MEMBER(str) Member(#str)

	constexpr const char* VALUE = "Value";

	constexpr const char* SingletonsField = "singletons";
	constexpr const char* EntitiesField = "entities";

	ECSCORE_API void FromJsonAsset(flecs::world& world, const FString name, const FString scope = TEXT(""));

	ECSCORE_API FString AddScope(const FString& in, const FString& scope = TEXT(""));
};