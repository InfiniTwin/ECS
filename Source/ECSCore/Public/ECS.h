// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"

namespace ECS {
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

	ECSCORE_API void FromJsonAsset(flecs::world& world, const FString name, const FString scope = TEXT(""));

	FString AddScope(const FString& in, const FString& scope = TEXT(""));

	static inline void EntitiesFromJson(flecs::world& world, TSharedPtr<FJsonObject>& root, const FString path);
	static inline void OverridesFromJson(flecs::world& world, flecs::entity entity, TSharedPtr<FJsonObject> entityObject);
	static inline void ChildrenFromJson(flecs::world& world, const TSharedPtr<FJsonObject>& entityObject, const FString& parentPath, const flecs::string_view& parentName);

	static inline void GetInstances(flecs::world& world, flecs::entity prefab, TArray<flecs::entity>& instances)
	{
		TQueue<flecs::entity> queue;
		queue.Enqueue(prefab);
		TSet<uint64> seen;

		while (!queue.IsEmpty()) {
			flecs::entity current;
			queue.Dequeue(current);

			world.each(world.pair(flecs::IsA, current), [&](flecs::entity instance) {
				if (instance.has(flecs::Prefab) && instance.is_a(prefab))
					queue.Enqueue(instance);
				else {
					uint64 id = instance.id();
					if (!seen.Contains(id)) {
						seen.Add(id);
						instances.Add(instance);
					}
				}
				});
		}
	}

	static inline void SingletonsFromJson(flecs::world& world, TSharedPtr<FJsonObject>& root)
	{
		const TSharedPtr<FJsonObject>* singletonsObject = nullptr;
		if (root->TryGetObjectField("singletons", singletonsObject)) {
			FString singletonsJsonString;
			FJsonSerializer::Serialize(singletonsObject->ToSharedRef(),
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
	}

	static inline void EntitiesFromJson(flecs::world& world, TSharedPtr<FJsonObject>& root, const FString path) {
		const TArray<TSharedPtr<FJsonValue>>* entities = nullptr;
		if (root->TryGetArrayField("entities", entities))
			for (const TSharedPtr<FJsonValue>& entityValue : *entities) {
				const TSharedPtr<FJsonObject>* entityObject = nullptr;
				if (entityValue->TryGetObject(entityObject)) {
					// Prepend "parent" field
					TSharedPtr<FJsonObject> modifiedEntityObject = MakeShared<FJsonObject>();
					modifiedEntityObject->SetStringField(TEXT("parent"), path);
					for (const auto& kvp : (*entityObject)->Values)
						modifiedEntityObject->SetField(kvp.Key, kvp.Value);

					FString entityJsonString;
					FJsonSerializer::Serialize(modifiedEntityObject.ToSharedRef(), TJsonWriterFactory<>::Create(&entityJsonString));
					std::string entityJson = TCHAR_TO_UTF8(*entityJsonString);

					flecs::entity entity = world.entity();
					entity.from_json(entityJson.c_str());

					OverridesFromJson(world, entity, *entityObject);
					ChildrenFromJson(world, *entityObject, path, entity.name());
				}
			}
	}

	static inline void OverridesFromJson(flecs::world& world, flecs::entity entity, TSharedPtr<FJsonObject> entityObject) {
		const TArray<TSharedPtr<FJsonValue>>* overrides = nullptr;
		if (entityObject->TryGetArrayField("overrides", overrides))
			for (const TSharedPtr<FJsonValue>& overrideValue : *overrides) {
				const TSharedPtr<FJsonObject>* overrideObject = nullptr;
				if (overrideValue->TryGetObject(overrideObject)) {
					FString path;
					if ((*overrideObject)->TryGetStringField("path", path)) {
						const TSharedPtr<FJsonObject>* componentsObject = nullptr;
						if ((*overrideObject)->TryGetObjectField("components", componentsObject)) {
							TSharedPtr<FJsonObject> resultObject = MakeShared<FJsonObject>();
							resultObject->SetObjectField(TEXT("components"), *componentsObject);

							FString componentsJsonString;
							FJsonSerializer::Serialize(resultObject.ToSharedRef(), TJsonWriterFactory<>::Create(&componentsJsonString));
							std::string componentsJson = TCHAR_TO_UTF8(*componentsJsonString);

							auto overrider = world.entity().disable();
							overrider.from_json(componentsJson.c_str());
							flecs::entity overriden = entity.lookup(TCHAR_TO_UTF8(*path));
							overrider.each([&](flecs::id id) {
								const ecs_type_info_t* info = ecs_get_type_info(world, id);
								const void* ptr = overrider.get(id);
								ecs_set_id(world, overriden, id, info->size, ptr);
								});
						}
					}
				}
			}
	}

	static inline void ChildrenFromJson(flecs::world& world, const TSharedPtr<FJsonObject>& entityObject, const FString& parentPath, const flecs::string_view& parentName) {
		const TArray<TSharedPtr<FJsonValue>>* children = nullptr;
		if (entityObject->TryGetArrayField(TEXT("children"), children))
			if (children && children->Num() > 0) {
				TSharedPtr<FJsonObject> childrenRoot = MakeShared<FJsonObject>();
				childrenRoot->SetArrayField(TEXT("entities"), *children);
				EntitiesFromJson(world, childrenRoot, parentPath + TEXT(".") + UTF8_TO_TCHAR(parentName.c_str()));
			}
	}
}