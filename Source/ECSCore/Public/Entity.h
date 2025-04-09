// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>

struct ECSCORE_API Entity {
public:
	static void FromAsset(flecs::world& world, const FString key);
};