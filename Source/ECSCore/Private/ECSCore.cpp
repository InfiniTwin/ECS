// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECSCore.h"
#include "ECS.h"
#include "ActionFeature.h"

#define LOCTEXT_NAMESPACE "FECSCoreModule"

void FECSCoreModule::StartupModule() {}

void FECSCoreModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FECSCoreModule, ECSCore)

namespace ECS {
	Core::Core(flecs::world& world) {
		world.module<Core>();

		Scopes.Add(TEXT("[CORE]"), ECSCoreScope);

		ECS::RegisterOpaqueTypes(world);

		ActionFeature::RegisterComponents(world);
		ActionFeature::CreateObservers(world);
		ActionFeature::CreateSystems(world);
	}
}