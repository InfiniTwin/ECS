// Fill out your copyright notice in the Description page of Project Settings.

#include "ActionFeature.h"
#include "flecs.h"
#include "ECS.h"

namespace ECS {
	void ActionFeature::RegisterComponents(flecs::world& world) {
		using namespace ECS;
		world.component<Action>().add(flecs::CanToggle);
		world.component<Invert>().add(flecs::CanToggle);
		world.component<Operation>().add(flecs::Exclusive);

		world.component<Target>().member<FString>(VALUE).add(flecs::OnInstantiate, flecs::Inherit);
		world.component<Code>().member<FString>(VALUE).add(flecs::OnInstantiate, flecs::Inherit);

		world.component<Singletons>().add(flecs::OnInstantiate, flecs::Inherit);
		world.component<Tags>().add(flecs::OnInstantiate, flecs::Inherit);
		world.component<Components>().add(flecs::OnInstantiate, flecs::Inherit);
		world.component<Pairs>().add(flecs::OnInstantiate, flecs::Inherit);
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
		world.observer<Target>("SetActionTarget")
			.event(flecs::OnSet)
			.each([](flecs::entity action, Target& target) {
			FString path = UTF8_TO_TCHAR(action.parent().path().c_str());
			FString targetValue = target.Value;

			// Go up
			int32 ltIndex;
			while ((ltIndex = targetValue.Find(TEXT("<"))) != INDEX_NONE)
			{
				targetValue.RemoveAt(ltIndex, 1, false);
				int32 lastSepIndex = path.Find(TEXT("::"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				if (lastSepIndex != INDEX_NONE)
					path = path.Left(lastSepIndex);
				else
				{
					path.Empty();
					break;
				}
			}

			// Add child/ren
			if (!targetValue.IsEmpty())
				path += TEXT("::") + targetValue;

			target.Value = ECS::NormalizedPath(path);
				});

		world.system<>("TriggerAction")
			.with<Operation>().second(flecs::Wildcard)
			.with<Action>().id_flags(flecs::TOGGLE).with<Action>()
			.each([&world](flecs::entity action) {
			action.disable<Action>();
			bool add = action.has(Operation::Add);
			TriggerAction(world, action, add);
				});

		world.system<>("TriggerInverseAction")
			.with<Operation>().second(flecs::Wildcard)
			.with<Invert>().id_flags(flecs::TOGGLE).with<Invert>()
			.each([&world](flecs::entity action) {
			action.disable<Invert>();
			bool remove = !action.has(Operation::Add);
			TriggerAction(world, action, remove);
				});
	}
}