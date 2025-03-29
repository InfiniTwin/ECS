// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>

#define NAMEOF_COMPONENT(T) ([] { return #T; }())
#define NAMEOF_MEMBER(member) ([] { constexpr std::string_view name = #member; return name.substr(name.find_last_of(':') + 1); }().data())

struct ECSCORE_API Component {
public:
	static void DeserializeSingletons(flecs::world& world, const FString key);
};