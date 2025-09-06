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

		ECS::Scopes.Add(TEXT("[IsA]"), IsARelationship);
		ECS::Scopes.Add(TEXT("[OrderedTrait]"), OrderedChildrenTrait);

		ECS::Scopes.Add(TEXT("[CORE]"), ECSCoreScope);

		RegisterOpaqueTypes(world);

		ActionFeature::CreateComponents(world);
		ActionFeature::CreateObservers(world);
		ActionFeature::CreateSystems(world);
	}

	void RegisterOpaqueTypes(flecs::world& world) {
		world.component<FString>()
			.opaque(flecs::String)
			.serialize([](const flecs::serializer* s, const FString* data) {
			const char* str = TCHAR_TO_UTF8(**data);
			return s->value(flecs::String, &str);
		})
			.assign_string([](FString* data, const char* value) {
			*data = UTF8_TO_TCHAR(value);
		});

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

		world.component<FVector2D>(MEMBER(FVector2D))
			.member<double>(MEMBER(FVector2D::X))
			.member<double>(MEMBER(FVector2D::Y));

		world.component<FVector>(MEMBER(FVector))
			.member<double>(MEMBER(FVector::X))
			.member<double>(MEMBER(FVector::Y))
			.member<double>(MEMBER(FVector::Z));

		world.component<FRotator>(MEMBER(FRotator))
			.member<double>(MEMBER(FRotator::Pitch))
			.member<double>(MEMBER(FRotator::Yaw))
			.member<double>(MEMBER(FRotator::Roll));

		world.component<FMargin>(MEMBER(FMargin))
			.member<float>(MEMBER(FMargin::Left))
			.member<float>(MEMBER(FMargin::Top))
			.member<float>(MEMBER(FMargin::Right))
			.member<float>(MEMBER(FMargin::Bottom));

		world.component<FLinearColor>(MEMBER(FLinearColor))
			.member<float>(MEMBER(FLinearColor::R))
			.member<float>(MEMBER(FLinearColor::G))
			.member<float>(MEMBER(FLinearColor::B))
			.member<float>(MEMBER(FLinearColor::A));
	}
}