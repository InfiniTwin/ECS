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

	ECSCORE_API extern TMap<FString, FString> Tokens;

	static inline FString SetScopes(const FString& data) {
		FString result = data;
		for (const auto& Pair : Tokens)
			result = result.Replace(*Pair.Key, *Pair.Value, ESearchCase::CaseSensitive);
		return result;
	}

	ECSCORE_API void RunCode(flecs::world& world, const FString& name, const FString& code);

	using namespace Assets;
	template<typename... Args>
	inline void RunScript(flecs::world& world, Args... args) {
		FString path = GetAssetPath(FlecsExtension, args...);
		FString code;
		if (FFileHelper::LoadFileToString(code, *path))
			RunCode(world, path, *code);
	}

	ECSCORE_API void RunScripts(flecs::world& world, const FString& folder, const TArray<FString>& files);

	static inline FString FullPath(const FString& path) {
		FString result = path;
		result.ReplaceInline(TEXT("."), TEXT("::"));
		if (!result.StartsWith(TEXT("::")))
			result = TEXT("::") + result;
		return result;
	}

	static inline FString NormalizedPath(const FString& path) {
		FString result = path;
		if (result.StartsWith(TEXT("::")))
			result.RightChopInline(2);
		result.ReplaceInline(TEXT("::"), TEXT("."));
		return result;
	}

	static inline void GetInstances(
		flecs::world& world,
		const flecs::entity prefab,
		TArray<flecs::entity>& instances) {
		TQueue<flecs::entity> queue;
		queue.Enqueue(prefab);
		TSet<uint64> seen;

		while (!queue.IsEmpty()) {
			flecs::entity current;
			queue.Dequeue(current);
			world.each(world.pair(flecs::IsA, current), [&](flecs::entity instance) {
				if (instance.has(flecs::Prefab) && instance.is_a(prefab))
					queue.Enqueue(instance);
				else {
					uint64 id = instance.id();
					if (!seen.Contains(id)) {
						seen.Add(id);
						instances.Add(instance);
					}
				}
				});
		}
	}

	static inline flecs::entity FirstChild(const flecs::entity& parent) {
		flecs::entity result{};
		parent.children([&](flecs::entity child) { result = child; });
		return result;
	}
}
