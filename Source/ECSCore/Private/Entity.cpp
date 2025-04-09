// Fill out your copyright notice in the Description page of Project Settings.


#include "Entity.h"
#include "Assets.h"

void Entity::FromAsset(flecs::world& world, const FString key) {
	auto data = Assets::LoadJsonAsset(key);

    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(data);
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
                UE_LOG(LogTemp, Log, TEXT("Element %d >>> %s"), index + 1, *FString(ElementString));
                index++;
            }
        }
        else {
            UE_LOG(LogTemp, Warning, TEXT(">>> Root is not an array."));
        }
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT(">>> Failed to deserialize JSON text."));
    }

	free(data);
}
