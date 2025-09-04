// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "flecs.h"
#include "ECS.h"

namespace ECS {
	struct ActionFeature {
		static void CreateComponents(flecs::world& world);
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
	struct Script { FString Value; }; // Value is script path relative to Assets folder

	static inline FString GetTarget(flecs::entity action) {
		FString path = UTF8_TO_TCHAR(action.parent().path().c_str());
		FString targetValue = action.try_get<Target>()->Value;
		int32 ltIndex;
		while ((ltIndex = targetValue.Find(TEXT("<"))) != INDEX_NONE) { // Go up
			targetValue.RemoveAt(ltIndex, 1, EAllowShrinking::No);
			int32 lastSepIndex = path.Find(TEXT("::"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			if (lastSepIndex != INDEX_NONE)
				path = path.Left(lastSepIndex);
			else {
				path.Empty();
				break;
			}
		}
		if (!targetValue.IsEmpty()) // Add child/ren
			path += TEXT("::") + targetValue;
		return ECS::FullPath(path);
	}

	static inline void FormatCode(FString& code) {
		code.ReplaceInline(TEXT("\\n"), TEXT("\n"));
		code.ReplaceInline(TEXT(" | "), TEXT("\n\t"));
		code.ReplaceInline(TEXT("<"), TEXT("{"));
		code.ReplaceInline(TEXT(">"), TEXT("}"));
		code += TEXT("\n}");
	}

	static inline void SetSingletons(flecs::world& world, flecs::entity action) {
		FString code = action.try_get<Code>()->Value;
		FormatCode(code);
		RunCode(world, "Set Singletons", code);
	}

	static inline void AddComponents(flecs::world& world, flecs::entity action) {
		FString code = NormalizedPath(GetTarget(action));
		code += TEXT(" {");
		code += action.try_get<Code>()->Value;
		FormatCode(code);
		RunCode(world, "Add Components", code);
	}

	static inline void RemoveComponents(flecs::world& world, flecs::entity action) {
		flecs::entity target = world.lookup(TCHAR_TO_UTF8(*GetTarget(action)));

		// Parse components
		TArray<FString> components;
		TArray<FString> parts;
		action.try_get<Code>()->Value.ParseIntoArray(parts, TEXT(" | "), true);
		for (int32 i = 0; i < parts.Num(); ++i) {
			const FString& part = parts[i];
			int32 colonIndex;
			if (part.FindChar(TEXT(':'), colonIndex))
				components.Add(part.Left(colonIndex).TrimStartAndEnd());
			else
				components.Add(part.TrimStartAndEnd());
		}

		// Remove components
		for (const FString& component : components)
			ecs_remove_id(world, target,
				ecs_id_from_str(world, TCHAR_TO_UTF8(*NormalizedPath(component))));
	}

	static inline void UpdatePairs(flecs::world& world, flecs::entity action, bool add) {
		flecs::entity target = world.lookup(TCHAR_TO_UTF8(*GetTarget(action)));

		// Parse pairs
		TMap<FString, FString> pairs;
		TArray<FString> pairStrings;
		action.try_get<Code>()->Value.ParseIntoArray(pairStrings, TEXT(";"), true);
		for (const FString& pair : pairStrings) {
			FString key, value;
			if (pair.Split(TEXT(","), &key, &value)) {
				value = value.TrimQuotes();
				pairs.Add(key, value);
			}
		}

		// Take actions
		for (const TPair<FString, FString>& pair : pairs) {
			auto first = world.lookup(TCHAR_TO_UTF8(*FullPath(pair.Key))).id();
			auto second = world.lookup(TCHAR_TO_UTF8(*FullPath(pair.Value))).id();

			if (add) ecs_add_pair(world, target, first, second);
			else ecs_remove_pair(world, target, first, second);
		}
	}

	static inline void SwitchScript(flecs::world& world, flecs::entity action, bool add) {
		auto script = action.try_get<Script>();
		if (script->Value.IsEmpty())
			return;

		auto target = GetTarget(action);

		if (add)
			RunScript(world, "", script->Value,
				Tokens({ TOKEN(TOKEN_PATH, ECS::NormalizedPath(target)) }));
		else
			ClearScript(world, "", script->Value,
				Tokens({ TOKEN(TOKEN_PATH, target) }));
	}

	static inline void TriggerAction(flecs::world& world, flecs::entity action, bool add) {
		if (action.has<Singletons>())
			SetSingletons(world, action);
		else if (action.has<Components>())
			if (add) AddComponents(world, action);
			else RemoveComponents(world, action);
		else if (action.has<Pairs>())
			UpdatePairs(world, action, add);
		else if (action.has<Script>())
			SwitchScript(world, action, add);
	}
}