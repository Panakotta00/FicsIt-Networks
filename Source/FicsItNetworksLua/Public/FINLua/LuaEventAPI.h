#pragma once

#include "CoreMinimal.h"
#include "FINEventFilter.h"
#include "FINSignalData.h"
#include "FIRTrace.h"
#include "LuaUtil.h"
#include "LuaEventAPI.generated.h"

USTRUCT()
struct FFINLuaEvent {
	GENERATED_BODY()

	UPROPERTY()
	FFIRTrace Sender;

	UPROPERTY()
	FFINSignalData Data;
};

USTRUCT()
struct FFINLuaEventQueue {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFINLuaEvent> Events;

	UPROPERTY()
	FFINEventFilterExpression Filter;

	UPROPERTY()
	class UFINLuaProcessor* Processor = nullptr;

	UPROPERTY()
	int64 Key = 0;

	FFINLuaEventQueue() = default;
	FFINLuaEventQueue(class UFINLuaProcessor* Processor, int64 Key);
	FFINLuaEventQueue(const FFINLuaEventQueue& Other);
	FFINLuaEventQueue& operator=(const FFINLuaEventQueue& Other);
	~FFINLuaEventQueue();

	void AddEvent(const FFIRTrace& Sender, const FFINSignalData& Data) {
		if (Events.Num() >= 250) return;
		Events.Add(FFINLuaEvent{Sender, Data});
	}

	static void CollectReferences(void* Obj, FReferenceCollector& Collector);
};

USTRUCT()
struct FFINLuaEventRegistry {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TMap<int64, FFINEventFilterExpression> EventListeners;

	UPROPERTY(SaveGame)
	TMap<int64, FFINEventFilterExpression> EventQueues;

	UPROPERTY(SaveGame)
	TMap<int64, FFINEventFilterExpression> OneShots;

	static int64 FindNextKey(const TMap<int64, FFINEventFilterExpression>& registry) {
		int64 key = registry.Num();
		while (registry.Contains(key)) {
			key += 1;
		}
		return key;
	}
};

namespace FINLua {

	typedef TSharedRef<FFINLuaEventQueue> FEventQueue;
	FEventQueue luaFIN_pushEventQueue(lua_State* L, int64 key);
	FEventQueue* luaFIN_toEventQueue(lua_State* L, int index);
	FEventQueue& luaFIN_checkEventQueue(lua_State* L, int index);

	/**
	 * Tries to get global event registry and pushes it onto the stack.
	 * @param L the lua state
	 * @return a reference to the registry
	 */
	TSharedPtr<FFINLuaEventRegistry> luaFIN_getEventRegistry(lua_State* L);

	void luaFIN_handleEvent(lua_State* L, const FFIRTrace& sender, const FFINSignalData& data);
}
