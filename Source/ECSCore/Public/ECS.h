// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"

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

constexpr const char* SingletonsField = "singletons";
constexpr const char* EntitiesField = "entities";
constexpr const char* ParentField = "parent";
constexpr const char* ChildrenField = "children";

struct ECS {
public:
	static inline void FromJsonAsset(flecs::world& world, const FString name) {
		auto data = Assets::LoadJsonAsset(name);
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(data);
		free(data);

		TSharedPtr<FJsonObject> rootObject;
		if (!FJsonSerializer::Deserialize(reader, rootObject) || !rootObject.IsValid())
			return;

		// Create singletons
		const TSharedPtr<FJsonObject>* singletons = nullptr;
		if (rootObject->TryGetObjectField(SingletonsField, singletons)) {
			FString singletonsJsonString;
			FJsonSerializer::Serialize(singletons->ToSharedRef(),
				TJsonWriterFactory<>::Create(&singletonsJsonString));
			std::string singletonsJson = TCHAR_TO_UTF8(*singletonsJsonString);

			auto singletonsEntity = world.entity().disable();
			singletonsEntity.from_json(singletonsJson.c_str());

			singletonsEntity.each([&](flecs::id id) {
				const ecs_type_info_t* info = ecs_get_type_info(world, id);
				const void* ptr = singletonsEntity.get(id);
				ecs_set_id(world, world.entity(id), id, info->size, ptr);
				});

			singletonsEntity.destruct();
		}

		// Create entities
		const TArray<TSharedPtr<FJsonValue>>* entities = nullptr;
		if (rootObject->TryGetArrayField(EntitiesField, entities)) {
			int32 index = 0;
			for (const TSharedPtr<FJsonValue>& entity : *entities) {
				FString entityJsonString;
				FJsonSerializer::Serialize(entity->AsObject().ToSharedRef(),
					TJsonWriterFactory<>::Create(&entityJsonString));
				std::string entityJson = TCHAR_TO_UTF8(*entityJsonString);
				world.set_scope(flecs::entity());
				world.entity().from_json(entityJson.c_str());
				index++;
			}
		}
	}
};