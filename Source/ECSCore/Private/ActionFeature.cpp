// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionFeature.h"
#include "flecs.h"
#include "ECS.h"

namespace ECS {
	void ActionFeature::RegisterComponents(flecs::world& world) {
		using namespace ECS;
		world.component<Action>().add(flecs::OnInstantiate, flecs::Inherit);
		world.component<Path>().member<FString>(VALUE).add(flecs::OnInstantiate, flecs::Inherit);
		world.component<Parent>().member<FString>(VALUE).add(flecs::OnInstantiate, flecs::Inherit);
	}

	void ActionFeature::CreateObservers(flecs::world& world) {
	}
}