// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"
#include <string>

struct ECSCORE_API World {
public:
	static void DeserializeSingletons(flecs::world& world, const FString key);
};