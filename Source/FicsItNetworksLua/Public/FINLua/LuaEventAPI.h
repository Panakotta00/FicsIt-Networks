#pragma once

#include "CoreMinimal.h"
#include "FINEventFilter.h"
#include "FINLuaReferenceCollector.h"
#include "FINSignalData.h"
#include "FIRTrace.h"
#include "LuaUtil.h"
#include "LuaEventAPI.generated.h"

struct FFINLuaReferenceCollector;

USTRUCT()
struct FFINLuaEvent {
	GENERATED_BODY()

	UPROPERTY()
	FFIRTrace Sender;

	UPROPERTY()
	FFINSignalData Data;
};

USTRUCT()
struct FFINLuaEventQueue : public FFINLuaReferenceCollected {
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFINLuaEvent> Events;

	UPROPERTY()
	FFINEventFilterExpression Filter;

	UPROPERTY()
	int64 Key = 0;

	FFINLuaEventQueue() = default;
	FFINLuaEventQueue(FFINLuaReferenceCollector* ReferenceCollector, int64 Key);

	void AddEvent(const FFIRTrace& Sender, const FFINSignalData& Data) {
		if (Events.Num() >= 250) return;
		Events.Add(FFINLuaEvent{Sender, Data});
	}

	virtual void CollectReferences(FReferenceCollector& Collector) override;
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
	UPROPERTY(SaveGame)
	TMap<int64, int> OneShots_Futures;

	static int64 FindNextKey(const TMap<int64, FFINEventFilterExpression>& registry) {
		int64 key = registry.Num();
		while (registry.Contains(key)) {
			key += 1;
		}
		return key;
	}
};

struct IFINLuaEventSystem {
	virtual ~IFINLuaEventSystem() = default;

	virtual void Ignore(UObject* Object) = 0;
	virtual void IgnoreAll() = 0;
	virtual void Clear() = 0;
	virtual void Listen(const FFIRTrace& Object) = 0;
	virtual TArray<FFIRTrace> Listening() = 0;
	virtual TOptional<TTuple<FFIRTrace, FFINSignalData>> PullSignal() = 0;
	virtual double TimeSinceStart() = 0;
};

struct FFINLuaEventSystem : IFINLuaEventSystem {
	TDelegate<void(UObject*)> OnIgnore;
	TDelegate<void()> OnIgnoreAll;
	TDelegate<void()> OnClear;
	TDelegate<void(const FFIRTrace&)> OnListen;
	TDelegate<TArray<FFIRTrace>()> OnListening;
	TDelegate<TOptional<TTuple<FFIRTrace, FFINSignalData>>()> OnPullSignal;
	TDelegate<double()> OnTimeSinceStart;

	virtual void Ignore(UObject* Object) override { OnIgnore.Execute(Object); }
	virtual void IgnoreAll() override { OnIgnoreAll.Execute(); }
	virtual void Clear() override { OnClear.Execute(); }
	virtual void Listen(const FFIRTrace& Object) override { OnListen.Execute(Object); }
	virtual TArray<FFIRTrace> Listening() override { return OnListening.Execute(); }
	virtual TOptional<TTuple<FFIRTrace, FFINSignalData>> PullSignal() override { return OnPullSignal.Execute(); }
	virtual double TimeSinceStart() override { return OnTimeSinceStart.Execute(); }
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

	int luaFIN_pushEventData(lua_State* L, const FFIRTrace& sender, const FFINSignalData& data);
	void luaFIN_handleEvent(lua_State* L, const FFIRTrace& sender, const FFINSignalData& data);

	void luaFIN_setEventSystem(lua_State* L, IFINLuaEventSystem& EventSystem);
	IFINLuaEventSystem& luaFIN_getEventSystem(lua_State* L);
}
