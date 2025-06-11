// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "flecs.h"

class FECSCoreModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

namespace ECS {
	inline constexpr TCHAR ECSCoreScope[] = TEXT("ECS.Core");

	struct Core {
	public:
		ECSCORE_API Core(flecs::world& world);
	};
}