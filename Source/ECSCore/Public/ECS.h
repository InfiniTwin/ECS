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

constexpr const char* EntitiesField = "entities";
constexpr const char* ParentField = "parent";
constexpr const char* ChildrenField = "children";

struct ECS {
public:
	static inline void SingletonsFromAsset(flecs::world& world, const FString name) {
		auto data = Assets::LoadJsonAsset(name);
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

	static inline void EntitiesFromAsset(flecs::world& world, const FString name) {
		auto data = Assets::LoadJsonAsset(name);
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(data);
		free(data);

		// Create entities
		TSharedPtr<FJsonObject> rootObject;
		if (FJsonSerializer::Deserialize(reader, rootObject) && rootObject.IsValid()) {
			const TArray<TSharedPtr<FJsonValue>>* entities = nullptr;
			if (rootObject->TryGetArrayField(EntitiesField, entities)) {
				int32 index = 0;
				for (const TSharedPtr<FJsonValue>& entity : *entities) {
					FString entityJsonString;
					FJsonSerializer::Serialize(entity->AsObject().ToSharedRef(),
						TJsonWriterFactory<>::Create(&entityJsonString));
					std::string entityJson = TCHAR_TO_UTF8(*entityJsonString);
					world.entity().from_json(entityJson.c_str());
					index++;
				}
			}
		}

		// Create ChildOf relationships
		const TArray<TSharedPtr<FJsonValue>>* childOfArray = nullptr;
		if (rootObject->TryGetArrayField(MEMBER(flecs::ChildOf), childOfArray)) {
			for (const TSharedPtr<FJsonValue>& childOfValue : *childOfArray) {
				const TSharedPtr<FJsonObject>* childOfObject;
				childOfValue->TryGetObject(childOfObject);

				FString parentName;
				(*childOfObject)->TryGetStringField(ParentField, parentName);
				flecs::entity parent = world.lookup(TCHAR_TO_UTF8(*parentName));

				const TArray<TSharedPtr<FJsonValue>>* children = nullptr;
				(*childOfObject)->TryGetArrayField(ChildrenField, children);
				for (const TSharedPtr<FJsonValue>& childValue : *children) {
					FString childName;
					childValue->TryGetString(childName);
					world.lookup(TCHAR_TO_UTF8(*childName)).add(flecs::ChildOf, parent);
				}
			}
		}
	}
};