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
	struct Path { FString Value; };
	struct Parent { FString Value; };

	struct Data { FString Value; };

	struct SetSingletons {};

	void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}
	}

	static inline void UpdateSingletons(flecs::world& world, const FString& data)
	{
		FString code = data;
		code.ReplaceInline(TEXT("\\n"), TEXT("\n")); 
		code.ReplaceInline(TEXT("$"), TEXT("$ {\n\t"));
		code.ReplaceInline(TEXT(";"), TEXT("}\n\t"));
		code.ReplaceInline(TEXT(","), TEXT(": {"));
		code += TEXT("}\n}");
	
		RunScript(world, "Set Singletons", code);
	}
}