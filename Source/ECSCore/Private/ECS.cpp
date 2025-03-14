// Fill out your copyright notice in the Description page of Project Settings.


#include "ECS.h"

flecs::world* UECS::World() const { return world; }

void UECS::Initialize(FSubsystemCollectionBase& Collection)
{
	OnTickDelegate = FTickerDelegate::CreateUObject(this, &UECS::Tick);
	OnTickHandle = FTSTicker::GetCoreTicker().AddTicker(OnTickDelegate);

	char name[] = { "ECS" };
	char* argv = name;
	world = new flecs::world(1, &argv);

	// Flecs Explorer:  https://www.flecs.dev/explorer/
	World()->import<flecs::stats>();
	World()->set<flecs::Rest>({});

	UE_LOG(LogTemp, Warning, TEXT("ECS Subsystem initialized!"));
	Super::Initialize(Collection);
}

void UECS::Deinitialize()
{
	FTSTicker::GetCoreTicker().RemoveTicker(OnTickHandle);

	if (world)
	{
		delete world;
		world = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("ECS Subsystem deinitialized!"));
	Super::Deinitialize();
}

bool UECS::Tick(float DeltaTime)
{
	if (world) world->progress(DeltaTime);
	return true;
}
