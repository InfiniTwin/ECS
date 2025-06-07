// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"
#include <optional>

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

	enum LoadMode {
		Create,
		Edit,
		Destroy
	};

	ECSCORE_API void FromJsonAsset(flecs::world& world, const FString& path, const FString& scope, const FString& parent = TEXT(""), LoadMode mode = LoadMode::Create);

	FString AddScope(const FString& in, const FString& scope = TEXT(""));

	static inline FString FullPath(const FString& path) {
		FString result = path;
		result.ReplaceInline(TEXT("."), TEXT("::")); // Replace '.' with '::'		
		if (!result.StartsWith(TEXT("::")))  // Prepend '::' if not already present
			result = TEXT("::") + result;
		return result;
	}

	static inline FString NormalizedPath(const FString& path) {
		FString result = path;
		if (result.StartsWith(TEXT("::"))) // Remove leading "::" if present
			result.RightChopInline(2);
		result.ReplaceInline(TEXT("::"), TEXT(".")); // Replace all "::" with "."
		return result;
	}

	static inline void GetInstances(flecs::world& world, const flecs::entity prefab, TArray<flecs::entity>& instances)
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

	static inline void OverrideComponents(flecs::world& world, const TSharedPtr<FJsonObject>* root, std::optional<flecs::entity> overriden = std::nullopt)
	{
		const TSharedPtr<FJsonObject>* componentsObject = nullptr;
		if ((*root)->TryGetObjectField("components", componentsObject)) {
			for (const auto& pair : (*componentsObject)->Values) {
				const FString& name = pair.Key;
				const TSharedPtr<FJsonObject> data = pair.Value->AsObject();

				FString jsonString;
				TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&jsonString);
				FJsonSerializer::Serialize(data.ToSharedRef(), writer);
				writer->Close();

				std::string json = TCHAR_TO_UTF8(*jsonString);
				std::string fullName = TCHAR_TO_UTF8(*FullPath(name));
				flecs::id id = world.lookup(fullName.c_str());

				if (overriden.has_value()) // Override entity
					overriden.value().set_json(id, json.c_str());
				else // Override singleton
					world.entity(id).set_json(id, json.c_str());
			}
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
			OverrideComponents(world, singletonsObject);
		}
	}

	static inline void Override(flecs::world& world, const flecs::entity entity, const TSharedPtr<FJsonObject> entityObject) {
		const TArray<TSharedPtr<FJsonValue>>* overrides = nullptr;
		if (entityObject->TryGetArrayField("overrides", overrides))
			for (const TSharedPtr<FJsonValue>& overrideValue : *overrides) {
				const TSharedPtr<FJsonObject>* overridesObject = nullptr;
				if (overrideValue->TryGetObject(overridesObject)) {
					FString path;
					if ((*overridesObject)->TryGetStringField("path", path))
						OverrideComponents(world, overridesObject, entity.lookup(TCHAR_TO_UTF8(*path)));
				}
			}
	}

	static inline void EntitiesFromJson(flecs::world& world, TSharedPtr<FJsonObject>& root, const FString& parent, LoadMode mode) {
		const TArray<TSharedPtr<FJsonValue>>* entities = nullptr;
		if (root->TryGetArrayField("entities", entities))
			for (const TSharedPtr<FJsonValue>& entityValue : *entities) {
				const TSharedPtr<FJsonObject>* entityObject = nullptr;
				if (entityValue->TryGetObject(entityObject)) {
					TSharedPtr<FJsonObject> modifiedEntityObject = MakeShared<FJsonObject>();
					modifiedEntityObject->SetStringField(TEXT("parent"), parent); // Prepend "parent" field
					for (const auto& kvp : (*entityObject)->Values)
						modifiedEntityObject->SetField(kvp.Key, kvp.Value);

					auto name = (*entityObject)->GetStringField("name");
					auto entityPath = (parent + (name.IsEmpty() ? "" : TEXT(".") + name));

					flecs::entity entity;
					if (mode == LoadMode::Create)
					{
						entity = world.entity();
						FString entityJsonString;
						FJsonSerializer::Serialize(modifiedEntityObject.ToSharedRef(), TJsonWriterFactory<>::Create(&entityJsonString));
						std::string entityJson = TCHAR_TO_UTF8(*entityJsonString);
						entity.from_json(entityJson.c_str());
					}
					else
					{
						entity = world.lookup(TCHAR_TO_UTF8(*FullPath(entityPath)));
						if (!entity.is_valid())
						{
							UE_LOG(LogTemp, Error, TEXT("Entity not found: %s"), *FString(entityPath));
							continue;
						}
						if (mode == LoadMode::Destroy)
							entity.destruct();
					}

					Override(world, entity, *entityObject);
					EntitiesFromJson(world, modifiedEntityObject, entityPath, mode);
				}
			}
	}
}
