// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECS.h"

namespace ECS {
	void FromJsonAsset(flecs::world& world, const FString& path, const FString& scope, const FString& parent, LoadMode mode) {
		using namespace Assets;
		auto data = LoadJsonAsset(path);
		FString processedJson = (mode == LoadMode::Destroy)
			? data
			: AddScope(data, scope);
		free(data);

		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(processedJson);
		TSharedPtr<FJsonObject> root;
		if (!FJsonSerializer::Deserialize(reader, root) || !root.IsValid())
			return;

		auto previousScope = world.get_scope();
		world.set_scope(flecs::entity());

		if (mode != LoadMode::Destroy)
			SingletonsFromJson(world, root);

		EntitiesFromJson(world, root, parent.IsEmpty() ? scope : parent, mode);

		world.set_scope(previousScope);
	}
}

FString ECS::AddScope(const FString& in, const FString& scope) {
	FString plainPrefix = scope.EndsWith(TEXT(".")) ? scope.LeftChop(1) : scope;
	const FString dotPrefix = plainPrefix + TEXT(".");
	FString out = in;

	// Tags
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

	// Path entries: Replace "." with "::"
	cursor = 0;
	while (true) {
		cursor = out.Find(TEXT("\"path\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, cursor);
		if (cursor == INDEX_NONE) break;

		// Find the colon after "path"
		int32 colon = out.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromStart, cursor);
		if (colon == INDEX_NONE) break;

		// Find the starting quote of the value
		int32 valStart = colon + 1;
		while (valStart < out.Len() && FChar::IsWhitespace(out[valStart])) ++valStart;
		if (valStart >= out.Len() || out[valStart] != '"') { cursor = valStart + 1; continue; }
		++valStart;

		// Find the ending quote of the value
		int32 valEnd = valStart;
		while (valEnd < out.Len() && out[valEnd] != '"') {
			if (out[valEnd] == '\\') ++valEnd; // skip escaped characters
			++valEnd;
		}

		FString pathVal = out.Mid(valStart, valEnd - valStart);
		FString newPathVal = pathVal.Replace(TEXT("."), TEXT("::"));

		if (newPathVal != pathVal) {
			out = out.Left(valStart) + newPathVal + out.Mid(valEnd);
			int32 delta = newPathVal.Len() - pathVal.Len();
			valEnd += delta;
		}

		cursor = valEnd + 1;
	}

	// Pairs
	cursor = 0;
	while (true) {
		cursor = out.Find(TEXT("\"pairs\""), ESearchCase::IgnoreCase, ESearchDir::FromStart, cursor);
		if (cursor == INDEX_NONE) break;

		// Find opening {
		int32 open = out.Find(TEXT("{"), ESearchCase::CaseSensitive, ESearchDir::FromStart, cursor);
		if (open == INDEX_NONE) break;

		// Match the full {} block
		int32 depth = 1, i = open + 1;
		while (i < out.Len() && depth > 0) {
			if (out[i] == '{') depth++;
			else if (out[i] == '}') depth--;
			i++;
		}
		if (depth != 0) break;

		// Extract block
		int32 close = i - 1;
		FString block = out.Mid(open, close - open + 1);

		// Parse each "key": "value" entry inside the block
		FString updatedBlock = block;
		int32 innerPos = 0;
		while (true) {
			int32 keyStart = updatedBlock.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, innerPos);
			if (keyStart == INDEX_NONE) break;
			int32 keyEnd = updatedBlock.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, keyStart + 1);
			if (keyEnd == INDEX_NONE) break;

			FString key = updatedBlock.Mid(keyStart + 1, keyEnd - keyStart - 1);
			int32 colon = updatedBlock.Find(TEXT(":"), ESearchCase::CaseSensitive, ESearchDir::FromStart, keyEnd);
			if (colon == INDEX_NONE) break;

			// Find value
			int32 valStart = colon + 1;
			while (valStart < updatedBlock.Len() && FChar::IsWhitespace(updatedBlock[valStart])) valStart++;
			if (valStart >= updatedBlock.Len()) break;

			if (updatedBlock[valStart] == '"') {
				int32 valEnd = updatedBlock.Find(TEXT("\""), ESearchCase::CaseSensitive, ESearchDir::FromStart, valStart + 1);
				if (valEnd == INDEX_NONE) break;

				FString val = updatedBlock.Mid(valStart + 1, valEnd - valStart - 1);

				// Update key if needed
				if (!key.StartsWith(TEXT("flecs")) && !key.StartsWith(dotPrefix)) {
					FString newKey = dotPrefix + key;
					updatedBlock = updatedBlock.Left(keyStart + 1) + newKey + updatedBlock.Mid(keyEnd);
					int32 delta = newKey.Len() - key.Len();
					keyEnd += delta;
					colon += delta;
					valStart += delta;
					valEnd += delta;
				}

				// Update value if needed
				if (!val.StartsWith(TEXT("flecs")) && !val.StartsWith(dotPrefix)) {
					FString newVal = dotPrefix + val;
					updatedBlock = updatedBlock.Left(valStart + 1) + newVal + updatedBlock.Mid(valEnd);
					int32 delta = newVal.Len() - val.Len();
					valEnd += delta;
				}
				innerPos = valEnd + 1;
			}
			else {
				// Handle non-string values if needed (e.g., numbers)
				innerPos = valStart + 1;
			}
		}

		// Replace original block with updated block
		out = out.Left(open) + updatedBlock + out.Mid(close + 1);
		cursor = open + updatedBlock.Len();
	}

	return out;
}
