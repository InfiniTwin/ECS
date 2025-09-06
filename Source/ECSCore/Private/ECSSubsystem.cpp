// Fill out your copyright notice in the Description page of Project Settings.

#include "ECSSubsystem.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#endif

void UECSSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
	OnTickDelegate = FTickerDelegate::CreateUObject(this, &UECSSubsystem::Tick);
	OnTickHandle = FTSTicker::GetCoreTicker().AddTicker(OnTickDelegate);

	char name[] = { "ECS" };
	char* argv = name;
	World = MakeUnique<flecs::world>(1, &argv);

	// Flecs Explorer:  https://www.flecs.dev/explorer/
	World->import<flecs::stats>();
	World->set<flecs::Rest>({});

	Super::Initialize(Collection);
}

void UECSSubsystem::Deinitialize() {
	FTSTicker::GetCoreTicker().RemoveTicker(OnTickHandle);
	World.Reset();

	Super::Deinitialize();
}

bool UECSSubsystem::Tick(float DeltaTime) {
	World->progress(DeltaTime);
	return true;
}
