// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionFeature.h"
#include "flecs.h"
#include "ECS.h"

namespace ECS {
	void ActionFeature::RegisterComponents(flecs::world& world) {
		using namespace ECS;
		world.component<Action>().add(flecs::CanToggle);
		world.component<Invert>().add(flecs::CanToggle);
		world.component<Path>().member<FString>(VALUE).add(flecs::OnInstantiate, flecs::Inherit);
		world.component<Parent>().member<FString>(VALUE).add(flecs::OnInstantiate, flecs::Inherit);

		world.component<Data>().member<FString>(VALUE).add(flecs::OnInstantiate, flecs::Inherit);
		world.component<SetSingletons>().add(flecs::OnInstantiate, flecs::Inherit);
	}

	void ActionFeature::CreateObservers(flecs::world& world) {
		world.observer<>("SetupAction")
			.with<Action>()
			.event(flecs::OnAdd)
			.each([](flecs::entity entity) { entity.disable<Action>(); });

		world.observer<>("SetupInverseAction")
			.with<Invert>()
			.event(flecs::OnAdd)
			.each([](flecs::entity entity) { entity.disable<Invert>(); });
	}

	void ActionFeature::CreateSystems(flecs::world& world) {
		world.system<const Data>("TriggerAction")
			.with<Action>()
			.each([&world](flecs::entity entity, const Data data) {
			entity.disable<Action>();
			if (entity.has<SetSingletons>())
				UpdateSingletons(world, data.Value);
				});

		world.system<>("TriggerInverseAction")
			.with<Invert>()
			.each([](flecs::entity entity) {
			entity.disable<Invert>();
				});
	}
}