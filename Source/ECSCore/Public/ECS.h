// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"
#include <optional>

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

	template <typename Element, typename Vector = std::vector<Element>>
	flecs::opaque<Vector, Element> VectorReflection(flecs::world& world) {
		std::stringstream ss;
		ss << "vector_";
		ss << world.component<Element>().name();
		flecs::entity v = world.vector<Element>();
		v.set_name(ss.str().c_str());
		return flecs::opaque<Vector, Element>()
			.as_type(v)
			.serialize([](const flecs::serializer* s, const Vector* data) {
			for (const auto& el : *data)
				s->value(el);
			return 0; })
			.count([](const Vector* data) {
			return data->size();
				})
			.resize([](Vector* data, size_t size) {
			data->resize(size);
				})
			.ensure_element([](Vector* data, size_t elem) {
			if (data->size() <= elem)
				data->resize(elem + 1);
			return &data->data()[elem];
				});
	}

	void RegisterOpaqueTypes(flecs::world& world);

	ECSCORE_API extern TMap<FString, FString> Tokens;

	static inline FString SetScopes(const FString& data) {
		FString result = data;
		for (const auto& Pair : Tokens)
			result = result.Replace(*Pair.Key, *Pair.Value, ESearchCase::CaseSensitive);
		return result;
	}

	ECSCORE_API void RunScript(flecs::world& world, const FString& path);

	static inline void RunScript(flecs::world& world, const FString& name, const FString& code) {
		FString scopedCode = SetScopes(code);
		if (ecs_script_run(world, TCHAR_TO_ANSI(*name), TCHAR_TO_UTF8(*scopedCode)))
			UE_LOG(LogTemp, Error, TEXT(">>> Error Running Flecs Script: %s"), *name);
	}

	static inline FString FullPath(const FString& path) {
		FString result = path;
		result.ReplaceInline(TEXT("."), TEXT("::"));
		if (!result.StartsWith(TEXT("::")))
			result = TEXT("::") + result;
		return result;
	}

	static inline void GetInstances(
		flecs::world& world,
		const flecs::entity prefab,
		TArray<flecs::entity>& instances)
	{
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
		parent.children([&](flecs::entity child) {
			result = child;
			});
		return result;
	}

}
