// Fill out your copyright notice in the Description page of Project Settings.

#include "Component.h"
#include "Assets.h"

void Component::SingletonsFromAsset(flecs::world& world, const FString key) {
	auto data = Assets::LoadJsonAsset(key);

	auto singletonsEntity = world.entity().disable();
	singletonsEntity.from_json(data);
	free(data);

	singletonsEntity.each([&](flecs::id id) {
		const ecs_type_info_t* info = ecs_get_type_info(world, id);
		const void* ptr = singletonsEntity.get(id);
		ecs_set_id(world, world.entity(id), id, info->size, ptr);
		});

	singletonsEntity.destruct();
}
