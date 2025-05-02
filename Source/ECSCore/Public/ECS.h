// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <flecs.h>
#include "Assets.h"

#define COMPONENT(T) ([] { return #T; }())

constexpr const char* Member(const char* str) {
	const char* lastColon = str;
	for (const char* ptr = str; *ptr != '\0'; ++ptr)
		if (*ptr == ':')
			lastColon = ptr + 1;
	return lastColon;
}
#define MEMBER(str) Member(#str)

constexpr const char* VALUE = "Value";

constexpr const char* SingletonsField = "singletons";
constexpr const char* EntitiesField = "entities";
constexpr const char* ParentField = "parent";
constexpr const char* ChildrenField = "children";

struct ECS {
public:
	static inline void FromJsonAsset(flecs::world& world, const FString name, const FString scope = TEXT("")) {
		auto data = Assets::LoadJsonAsset(name);
		auto scoped = scope == "" ? data : ECS::AddScope(data, scope);
		free(data);
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(scoped);

		TSharedPtr<FJsonObject> rootObject;
		if (!FJsonSerializer::Deserialize(reader, rootObject) || !rootObject.IsValid())
			return;

		// Create singletons
		const TSharedPtr<FJsonObject>* singletons = nullptr;
		if (rootObject->TryGetObjectField(SingletonsField, singletons)) {
			FString singletonsJsonString;
			FJsonSerializer::Serialize(singletons->ToSharedRef(),
				TJsonWriterFactory<>::Create(&singletonsJsonString));
			std::string singletonsJson = TCHAR_TO_UTF8(*singletonsJsonString);

			auto singletonsEntity = world.entity().disable();
			singletonsEntity.from_json(singletonsJson.c_str());

			singletonsEntity.each([&](flecs::id id) {
				const ecs_type_info_t* info = ecs_get_type_info(world, id);
				const void* ptr = singletonsEntity.get(id);
				ecs_set_id(world, world.entity(id), id, info->size, ptr);
			});

			singletonsEntity.destruct();
		}

		// Create entities
		const TArray<TSharedPtr<FJsonValue>>* entities = nullptr;
		if (rootObject->TryGetArrayField(EntitiesField, entities)) {
			auto prevScope = world.get_scope();
			world.set_scope(flecs::entity());
			int32 index = 0;
			for (const TSharedPtr<FJsonValue>& entity : *entities) {
				FString entityJsonString;
				FJsonSerializer::Serialize(entity->AsObject().ToSharedRef(),
					TJsonWriterFactory<>::Create(&entityJsonString));
				std::string entityJson = TCHAR_TO_UTF8(*entityJsonString);
				world.entity().from_json(entityJson.c_str());
				index++;
			}
			world.set_scope(prevScope);
		}
	}

	static inline FString AddScope(const FString& in, const FString& scope = TEXT("")) {
		FString plainPrefix = scope.EndsWith(TEXT(".")) ? scope.LeftChop(1) : scope;
		const FString dotPrefix = plainPrefix + TEXT(".");
		FString out = in;

		int32 cursor = 0;
		while (true) {
			cursor = out.Find(TEXT("\"tags\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, cursor);
			if (cursor == INDEX_NONE) break;
			int32 open = out.Find(TEXT("["), ESearchCase::CaseSensitive, ESearchDir::FromStart, cursor);
			if (open == INDEX_NONE) break;
			int32 i = open + 1;
			while (true) {
				while (i < out.Len() && FChar::IsWhitespace(out[i])) ++i;
				if (i >= out.Len() || out[i] == ']') { ++i; break; }
				if (out[i] != '"') { ++i; continue; }
				int32 start = i + 1, end = start;
				while (end < out.Len() && out[end] != '"') { if (out[end] == '\\') ++end; ++end; }
				FString tag = out.Mid(start, end - start);
				bool scoped = tag.StartsWith(plainPrefix) && (tag.Len() == plainPrefix.Len() || tag.Mid(plainPrefix.Len(), 1) == TEXT("."));
				bool flecs = tag.StartsWith(TEXT("flecs"));
				if (!scoped && !flecs) {
					FString newTag = dotPrefix + tag;
					out = out.Left(start) + newTag + out.Mid(end);
					int32 delta = newTag.Len() - tag.Len();
					end += delta;
					i = end;
				} else i = end;
				++i;
			}
			cursor = i;
		}

		bool skippedSingletons = false;
		cursor = 0;
		while (true) {
			cursor = out.Find(TEXT("\"components\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, cursor);
			if (cursor == INDEX_NONE) break;
			if (!skippedSingletons) { skippedSingletons = true; cursor += 12; continue; }
			int32 open = out.Find(TEXT("{"), ESearchCase::CaseSensitive, ESearchDir::FromStart, cursor);
			if (open == INDEX_NONE) break;
			int32 depth = 1, i = open + 1;
			for (; i < out.Len() && depth > 0; ++i) {
				if (out[i] == '{') ++depth; else if (out[i] == '}') --depth;
				if (depth != 1) continue;
				if (FChar::IsWhitespace(out[i]) || out[i] != '"') continue;
				int32 start = i + 1, end = start;
				while (end < out.Len() && out[end] != '"') { if (out[end] == '\\') ++end; ++end; }
				FString key = out.Mid(start, end - start);
				int32 colon = out.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromStart, end);
				if (colon == INDEX_NONE) break;
				bool scoped = key.StartsWith(plainPrefix) && (key.Len() == plainPrefix.Len() || key.Mid(plainPrefix.Len(), 1) == TEXT("."));
				if (!scoped) {
					FString newKey = dotPrefix + key;
					out = out.Left(start) + newKey + out.Mid(end);
					int32 delta = newKey.Len() - key.Len();
					end += delta;
					i = colon + delta;
				} else i = colon;
			}
			cursor = i;
		}

		cursor = 0;
		while (true) {
			cursor = out.Find(TEXT("\"parent\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, cursor);
			if (cursor == INDEX_NONE) break;
			int32 quote = out.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, cursor + 8);
			if (quote == INDEX_NONE) break;
			while (quote < out.Len() && out[quote] != '"') ++quote;
			if (quote >= out.Len()) break;
			int32 valStart = quote + 1, valEnd = valStart;
			while (valEnd < out.Len() && out[valEnd] != '"') { if (out[valEnd] == '\\') ++valEnd; ++valEnd; }
			FString val = out.Mid(valStart, valEnd - valStart);
			bool scoped = val.StartsWith(plainPrefix) && (val.Len() == plainPrefix.Len() || val.Mid(plainPrefix.Len(), 1) == TEXT("."));
			if (!scoped) {
				FString newVal = val.IsEmpty() ? plainPrefix : dotPrefix + val;
				out = out.Left(valStart) + newVal + out.Mid(valEnd);
				int32 delta = newVal.Len() - val.Len();
				valEnd += delta;
				cursor = valEnd;
			} else cursor = valEnd;
		}

		return out;
	}
};