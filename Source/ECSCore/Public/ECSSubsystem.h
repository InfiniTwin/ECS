// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "flecs.h"
#include "ECSSubsystem.generated.h"

UCLASS()
class ECSCORE_API UECSSubsystem : public UGameInstanceSubsystem {
	GENERATED_BODY()

public:
	TUniquePtr<flecs::world> World;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	bool Tick(float DeltaTime);

protected:
	FTickerDelegate OnTickDelegate;
	FTSTicker::FDelegateHandle OnTickHandle;
};
