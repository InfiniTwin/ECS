// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "flecs.h"
#include "ECS.h"

namespace ECS {
	struct ActionFeature {
		static void RegisterComponents(flecs::world& world);
		static void CreateObservers(flecs::world& world);
		static void CreateSystems(flecs::world& world);
	};

	struct Action {};
	struct Invert {};
	enum Operation {
		Add,
		Remove
	};

	struct Target { FString Value; };
	struct Code { FString Value; };

	struct Singletons {};
	struct Tags {};
	struct Components {};
	struct Pairs {};

	void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	}

	static inline TMap<FString, FString> GetPairs(const FString& input)
	{
		TMap<FString, FString> result;

		TArray<FString> pairs;
		input.ParseIntoArray(pairs, TEXT(";"), true);

		for (const FString& pair : pairs)
		{
			FString key, value;
			if (pair.Split(TEXT(","), &key, &value))
			{
				value = value.TrimQuotes();
				result.Add(key, value);
			}
		}

		return result;
	}

	static inline void SetSingletons(flecs::world& world, const FString& data)
	{
		FString code = data;
		code.ReplaceInline(TEXT("\\n"), TEXT("\n"));
		code.ReplaceInline(TEXT("$"), TEXT("$ {\n\t"));
		code.ReplaceInline(TEXT(";"), TEXT("}\n\t"));
		code.ReplaceInline(TEXT(","), TEXT(": {"));
		code += TEXT("}\n}");

		RunScript(world, "Set Singletons", code);
	}

	static inline void UpdatePairs(flecs::world& world, flecs::entity action, bool add)
	{
		flecs::entity target = world.lookup(TCHAR_TO_UTF8(*action.get<Target>()->Value));

		for (const TPair<FString, FString>& pair : GetPairs(action.get<Code>()->Value))
		{
			auto key = TCHAR_TO_UTF8(*FullPath(pair.Key));
			auto value = TCHAR_TO_UTF8(*FullPath(pair.Value));

			auto first = world.lookup(TCHAR_TO_UTF8(*FullPath(pair.Key))).id();
			auto second = world.lookup(TCHAR_TO_UTF8(*FullPath(pair.Value))).id();

			if (add)
				ecs_add_pair(world, target, first, second);
			else
				ecs_remove_pair(world, target, first, second);
		}
	}
}