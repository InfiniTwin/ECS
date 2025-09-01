// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include <flecs.h>

class FECSCoreModule : public IModuleInterface {
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

namespace ECS {
	inline constexpr TCHAR IsARelationship[] = TEXT("flecs.core.IsA");
	inline constexpr TCHAR OrderedChildrenTrait[] = TEXT("flecs.core.OrderedChildren");
	
	inline constexpr TCHAR ECSCoreScope[] = TEXT("ECS.Core");

	struct Core {
	public:
		ECSCORE_API Core(flecs::world& world);
	};

	void RegisterOpaqueTypes(flecs::world& world);

	template <typename Element, typename Array = TArray<Element>>
	flecs::opaque<Array, Element> ArrayType(flecs::world& world) {
		flecs::entity array = world.vector<Element>();
		FString Name = FString::Printf(TEXT("OpaqueArray_%s"), *FString(world.component<Element>().name()));
		array.set_name(TCHAR_TO_ANSI(*Name));

		return flecs::opaque<Array, Element>()
			.as_type(array)
			.serialize([](const flecs::serializer* s, const Array* data) {
			for (const Element& el : *data)
				s->value(el);
			return 0;
		})
			.count(+[](const Array* data) -> size_t {
			return data->Num();
		})
			.resize([](Array* data, size_t size) {
			if (data)
				data->SetNum(static_cast<int32>(size));
		})
			.ensure_element([](Array* data, size_t elem) -> Element* {
			if (!data)
				return nullptr;
			if (data->Num() <= static_cast<int32>(elem))
				data->SetNum(elem + 1);
			return &(*data)[elem];
		});
	}
}