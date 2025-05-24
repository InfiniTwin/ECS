// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"

namespace ECS {
	void FromJsonAsset(flecs::world& world, const FString name, const FString scope) {
		using namespace Assets;
		auto data = LoadJsonAsset(name);
		auto scoped = scope == "" ? data : ECS::AddScope(data, scope);
		free(data);
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(scoped);

		TSharedPtr<FJsonObject> rootObject;
		if (!FJsonSerializer::Deserialize(reader, rootObject) || !rootObject.IsValid())
			return;

		auto prevScope = world.get_scope();
		world.set_scope(flecs::entity());

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
				auto idstr = id.str();
				const ecs_type_info_t* info = ecs_get_type_info(world, id);
				auto name = info->name;
				const void* ptr = singletonsEntity.get(id);
				ecs_set_id(world, world.entity(id), id, info->size, ptr);
				});

			singletonsEntity.destruct();
		}

		// Create entities
		const TArray<TSharedPtr<FJsonValue>>* entities = nullptr;
		if (rootObject->TryGetArrayField(EntitiesField, entities)) {
			int32 index = 0;
			for (const TSharedPtr<FJsonValue>& entity : *entities) {
				FString entityJsonString;
				FJsonSerializer::Serialize(entity->AsObject().ToSharedRef(),
					TJsonWriterFactory<>::Create(&entityJsonString));
				std::string entityJson = TCHAR_TO_UTF8(*entityJsonString);
				world.entity().from_json(entityJson.c_str());
				index++;
			}
		}
		
		world.set_scope(prevScope);
	}

	FString AddScope(const FString& in, const FString& scope) {
		FString plainPrefix = scope.EndsWith(TEXT(".")) ? scope.LeftChop(1) : scope;
		const FString dotPrefix = plainPrefix + TEXT(".");
		FString out = in;

		// Parent
		int32 cursor = 0;
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
			}
			else cursor = valEnd;
		}

		// Tags
		cursor = 0;
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
				}
				else i = end;
				++i;
			}
			cursor = i;
		}

		// IsA
		cursor = 0;
		while (true) {
			cursor = out.Find(TEXT("\"flecs.core.IsA\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, cursor);
			if (cursor == INDEX_NONE) break;
			int32 colon = out.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromStart, cursor);
			if (colon == INDEX_NONE) break;
			int32 nextChar = colon + 1;
			while (nextChar < out.Len() && FChar::IsWhitespace(out[nextChar])) ++nextChar;
			if (nextChar < out.Len() && out[nextChar] == '"') { // Single IsA entry
				int32 valStart = nextChar + 1, valEnd = valStart;
				while (valEnd < out.Len() && out[valEnd] != '"') { if (out[valEnd] == '\\') ++valEnd; ++valEnd; }

				FString val = out.Mid(valStart, valEnd - valStart);
				bool scoped = val.StartsWith(plainPrefix) && (val.Len() == plainPrefix.Len() || val.Mid(plainPrefix.Len(), 1) == TEXT("."));
				bool flecs = val.StartsWith(TEXT("flecs"));
				if (!scoped && !flecs) {
					FString newVal = dotPrefix + val;
					out = out.Left(valStart) + newVal + out.Mid(valEnd);
					int32 delta = newVal.Len() - val.Len();
					valEnd += delta;
				}
				cursor = valEnd + 1;
			}
			else if (nextChar < out.Len() && out[nextChar] == '[') { // Multiple IsA entries
				int32 i = nextChar + 1;
				while (true) {
					while (i < out.Len() && FChar::IsWhitespace(out[i])) ++i;
					if (i >= out.Len() || out[i] == ']') { ++i; break; }
					if (out[i] != '"') { ++i; continue; }

					int32 valStart = i + 1, valEnd = valStart;
					while (valEnd < out.Len() && out[valEnd] != '"') { if (out[valEnd] == '\\') ++valEnd; ++valEnd; }

					FString val = out.Mid(valStart, valEnd - valStart);
					bool scoped = val.StartsWith(plainPrefix) && (val.Len() == plainPrefix.Len() || val.Mid(plainPrefix.Len(), 1) == TEXT("."));
					bool flecs = val.StartsWith(TEXT("flecs"));
					if (!scoped && !flecs) {
						FString newVal = dotPrefix + val;
						out = out.Left(valStart) + newVal + out.Mid(valEnd);
						int32 delta = newVal.Len() - val.Len();
						valEnd += delta;
						i = valEnd;
					}
					else i = valEnd;
					++i;
				}
				cursor = i;
			}
			else cursor = nextChar + 1;
		}

		// Components
		cursor = 0;
		while (true) {
			cursor = out.Find(TEXT("\"components\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, cursor);
			if (cursor == INDEX_NONE) break;
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
				}
				else i = colon;
			}
			cursor = i;
		}

		return out;
	}
}