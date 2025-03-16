// Fill out your copyright notice in the Description page of Project Settings.


#include "ECS.h"

void UECS::Initialize(FSubsystemCollectionBase& Collection)
{
	OnTickDelegate = FTickerDelegate::CreateUObject(this, &UECS::Tick);
	OnTickHandle = FTSTicker::GetCoreTicker().AddTicker(OnTickDelegate);

	char name[] = { "ECS" };
	char* argv = name;
	World = MakeUnique<flecs::world>(1, &argv);

	// Flecs Explorer:  https://www.flecs.dev/explorer/
	World->import<flecs::stats>();
	World->set<flecs::Rest>({});

	Super::Initialize(Collection);
}

void UECS::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(OnTickHandle);
	World.Reset();

	Super::Deinitialize();
}

bool UECS::Tick(float DeltaTime)
{
	World->progress(DeltaTime);
	return true;
}
