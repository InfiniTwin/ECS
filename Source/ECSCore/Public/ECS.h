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
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(data);
        free(data);
        TSharedPtr<FJsonValue> RootValue;

        if (FJsonSerializer::Deserialize(Reader, RootValue)) {
            const TArray<TSharedPtr<FJsonValue>>* ArrayValue = nullptr;

            if (RootValue->TryGetArray(ArrayValue)) {
                int32 index = 0;
                for (const TSharedPtr<FJsonValue>& Element : *ArrayValue) {
                    FString JsonString;
                    FJsonSerializer::Serialize(Element->AsObject().ToSharedRef(), TJsonWriterFactory<>::Create(&JsonString));
                    const char* ElementString = TCHAR_TO_UTF8(*JsonString);
                    world.entity().from_json(ElementString);
                    index++;
                }
            }
        }
    }
};