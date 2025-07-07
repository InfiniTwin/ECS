// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECSCore.h"
#include "ECS.h"
#include "ActionFeature.h"

#define LOCTEXT_NAMESPACE "FECSCoreModule"

void FECSCoreModule::StartupModule() {}

void FECSCoreModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FECSCoreModule, ECSCore)

namespace ECS {
	Core::Core(flecs::world& world) {
		world.module<Core>();

		ECS::Tokens.Add(TEXT("[IsA]"), IsARelationship);
		ECS::Tokens.Add(TEXT("[CORE]"), ECSCoreScope);

		RegisterOpaqueTypes(world);

		ActionFeature::RegisterComponents(world);
		ActionFeature::CreateObservers(world);
		ActionFeature::CreateSystems(world);
	}

	void RegisterOpaqueTypes(flecs::world& world) {
		// FString <=> flecs::String
		world.component<FString>()
			.opaque(flecs::String)
			.serialize([](const flecs::serializer* s, const FString* data) {
			const char* str = TCHAR_TO_UTF8(**data);
			return s->value(flecs::String, &str);
				})
			.assign_string([](FString* data, const char* value) {
			*data = UTF8_TO_TCHAR(value);
				});

		// FText <=> flecs::String
		world.component<FText>()
			.opaque(flecs::String)
			.serialize([](const flecs::serializer* s, const FText* data) {
			FString temp = data->ToString();
			const char* str = TCHAR_TO_UTF8(*temp);
			return s->value(flecs::String, &str);
				})
			.assign_string([](FText* data, const char* value) {
			*data = FText::FromString(UTF8_TO_TCHAR(value));
				});

		world.component<FVector3f>()
			.member<float>(MEMBER(FVector3f::X))
			.member<float>(MEMBER(FVector3f::Y))
			.member<float>(MEMBER(FVector3f::Z));

		world.component<FVector4f>()
			.member<float>(MEMBER(FVector4f::X))
			.member<float>(MEMBER(FVector4f::Y))
			.member<float>(MEMBER(FVector4f::Z))
			.member<float>(MEMBER(FVector4f::Q));

		world.component<FMargin>()
			.member<float>(MEMBER(FMargin::Left))
			.member<float>(MEMBER(FMargin::Top))
			.member<float>(MEMBER(FMargin::Right))
			.member<float>(MEMBER(FMargin::Bottom));

		world.component<TArray<int>>().opaque(ArrayType<int>);
		world.component<TArray<float>>().opaque(ArrayType<float>);
		world.component<TArray<FVector4f>>().opaque(ArrayType<FVector4f>);
	}
}