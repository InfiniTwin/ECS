// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "flecs.h"

namespace ECS {
	struct ActionFeature {
		static void RegisterOpaqueTypes(flecs::world& world);
		static void RegisterComponents(flecs::world& world);
		static void CreateObservers(flecs::world& world);
	};

	struct Action {};
	struct Path { FString Value; };
	struct Parent { FString Value; };
}