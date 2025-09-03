// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"
#include "Containers/Queue.h"
#include "Containers/Array.h"

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

#define TOKEN(Key, Value) TPair<FString, FString>(Key, Value)

	constexpr const char* VALUE = "Value";

	inline constexpr TCHAR TOKEN_PATH[] = TEXT("[PATH]");
	inline constexpr TCHAR TOKEN_ID[] = TEXT("[ID]");
	inline constexpr TCHAR TOKEN_TARGET[] = TEXT("[TARGET]");

	ECSCORE_API extern TMap<FString, FString> Scopes;

	ECSCORE_API const TMap<FString, FString>& EmptyTokens();

	ECSCORE_API TMap<FString, FString> Tokens(const TArray<TPair<FString, FString>>& pairs);

	ECSCORE_API void RunCode(
		flecs::world& world,
		const FString& name,
		const FString& code,
		const TMap<FString, FString>& tokens = EmptyTokens());

	ECSCORE_API void RunScript(
		flecs::world& world,
		const FString& path,
		const FString& file,
		const TMap<FString, FString>& tokens = EmptyTokens());

	ECSCORE_API void RunScripts(
		flecs::world& world,
		const FString& path,
		const TArray<FString>& files,
		const TMap<FString, FString>& tokens = EmptyTokens());

	ECSCORE_API void ClearScript(
		flecs::world& world,
		const FString& path,
		const FString& file,
		const TMap<FString, FString>& tokens = EmptyTokens());

	static inline FString IdString(const flecs::entity_t id) {
		return "#" + LexToString(static_cast<uint32>(id));
	}

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

	static inline FString CleanCode(const FString& code) {
		FString clean = code;
		clean.ReplaceInline(TEXT("$"), TEXT("\\$"));
		return clean;
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

	static inline bool IsDescendant(flecs::entity descendant, flecs::entity ancestor) {
		for (auto parent = descendant.target(flecs::ChildOf); parent.is_valid(); parent = parent.target(flecs::ChildOf))
			if (parent == ancestor) return true;
		return false;
	}

	template <typename... C>
	TArray<flecs::entity> FindDescendants(flecs::entity ancestor, int32 maxDepth = -1) {
		static_assert(sizeof...(C) > 0, "FindDescendants needs at least one component type.");

		struct SearchEntry {
			flecs::entity entity;
			int32 depth;
		};

		TArray<flecs::entity> result;
		TQueue<SearchEntry> queue;

		ancestor.children([&](flecs::entity child) { queue.Enqueue({ child, 1 }); });

		SearchEntry current;
		while (queue.Dequeue(current)) {
			if ((current.entity.has<C>() && ...))
				result.Add(current.entity);
			if (maxDepth < 0 || current.depth < maxDepth)
				current.entity.children([&](flecs::entity child) { queue.Enqueue({ child, current.depth + 1 }); });
		}

		return result;
	}

	template <typename... C>
	TArray<flecs::entity> FindAncestors(flecs::entity descendant, int32 maxDepth = -1) {
		static_assert(sizeof...(C) > 0, "FindAncestors needs at least one component type.");

		TArray<flecs::entity> result;

		flecs::entity current = descendant.parent();
		int32 depth = 1;

		while (current.is_valid() && (maxDepth < 0 || depth <= maxDepth)) {
			if ((current.has<C>() && ...))
				result.Add(current);

			current = current.parent();
			depth++;
		}

		return result;
	}
}
