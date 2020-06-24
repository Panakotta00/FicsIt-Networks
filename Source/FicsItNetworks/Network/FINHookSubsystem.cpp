#include "FINHookSubsystem.h"

#include "FINNetworkTrace.h"
#include "FINSubsystemHolder.h"
#include "Signals/FINSignalListener.h"

TMap<UClass*, TSet<TSubclassOf<UFINHook>>> AFINHookSubsystem::HookRegistry;

AFINHookSubsystem* AFINHookSubsystem::GetHookSubsystem(UObject* WorldContext) {
	return GetSubsystemHolder<UFINSubsystemHolder>(WorldContext)->HookSubsystem;
}

void AFINHookSubsystem::RegisterHook(UClass* clazz, TSubclassOf<UFINHook> hook) {
	HookRegistry.FindOrAdd(clazz).Add(hook);
}

TSet<FFINNetworkTrace>& AFINHookSubsystem::GetListeners(UObject* object) {
	return Data[object].Listeners;
}

void AFINHookSubsystem::EmitSignal(UObject* object, FFINSignal signal) {
	for (FFINNetworkTrace trace : GetListeners(object)) {
		UObject* obj = *trace;
		if (IsValid(obj)) IFINSignalListener::Execute_HandleSignal(obj, signal, trace.getTrace().reverse());
	}
}

void AFINHookSubsystem::AttachHooks(UObject* object) {
	ClearHooks(object);
	TSet<UFINHook*>& hooks = Data.FindOrAdd(object).Hooks;
	UClass* clazz = object->GetClass();
	while (clazz) {
		TSet<TSubclassOf<UFINHook>>* hookClasses = HookRegistry.Find(clazz);
		if (hookClasses) for (TSubclassOf<UFINHook> hookClass : *hookClasses) {
			UFINHook* hook = NewObject<UFINHook>(this, hookClass);
			hooks.Add(hook);
			hook->Register(object);
		}
		
		if (clazz == UObject::StaticClass()) clazz = nullptr;
		else clazz = clazz->GetSuperClass();
	}
	for (UFINHook* hook : hooks) {
		hook->Register(object);
	}
}

void AFINHookSubsystem::ClearHooks(UObject* object) {
	FFINHookData* data = Data.Find(object);
	if (!data) return;
	for (UFINHook* hook : data->Hooks) {
		hook->Unregister();
	}
	data->Hooks.Empty();
}

void AFINHookSubsystem::AddListener(UObject* sender, FFINNetworkTrace listener) {
	AttachHooks(sender);
	Data[sender].Listeners.Add(listener);
}

void AFINHookSubsystem::RemoveListener(UObject* sender, UObject* listener) {
	TSet<FFINNetworkTrace> traces = Data[sender].Listeners;
	traces.Remove(FFINNetworkTrace(listener));
	if (traces.Num() < 1) ClearHooks(sender);
}

