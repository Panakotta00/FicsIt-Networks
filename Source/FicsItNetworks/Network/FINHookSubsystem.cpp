#include "FINHookSubsystem.h"

#include "FINNetworkTrace.h"
#include "FINSubsystemHolder.h"
#include "Signals/FINSignalListener.h"
#include "Signals/FINSignalSender.h"
#include "util/Logging.h"

TMap<UClass*, TSet<TSubclassOf<UFINHook>>> AFINHookSubsystem::HookRegistry;

bool FFINHookData::Serialize(FArchive& Ar) {
	Ar << Listeners;
	return true;
}

void AFINHookSubsystem::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
	Ar << Data;
	if (Ar.IsLoading()) {
		for (const TTuple<UObject*, FFINHookData>& data : Data) {
			AttachHooks(data.Key);
		}
		for (UClass* clazz : ClassesWithSignals) {
			UFINSignalUtility::SetupSender(clazz);
		}
	}
}

bool AFINHookSubsystem::ShouldSave_Implementation() const {
	return true;
}

AFINHookSubsystem* AFINHookSubsystem::GetHookSubsystem(UObject* WorldContext) {
	return GetSubsystemHolder<UFINSubsystemHolder>(WorldContext)->HookSubsystem;
}

void AFINHookSubsystem::RegisterHook(UClass* clazz, TSubclassOf<UFINHook> hook) {
	HookRegistry.FindOrAdd(clazz).Add(hook);
}

TSet<FFINNetworkTrace> AFINHookSubsystem::GetListeners(UObject* object) const {
	TSet<FFINNetworkTrace> traces;
	for (const FFINNetworkTrace& trace : Data[object].Listeners) {
		traces.Add(trace);
	}
	return traces;
}

void AFINHookSubsystem::EmitSignal(UObject* object, const TFINDynamicStruct<FFINSignal>& signal) {
	for (FFINNetworkTrace trace : GetListeners(object)) {
		IFINSignalListener* obj = Cast<IFINSignalListener>(*trace);
		if (obj) obj->HandleSignal(signal, trace.Reverse());
	}
}

void AFINHookSubsystem::AttachHooks(UObject* object) {
	if (!IsValid(object)) return;
	ClearHooks(object);
	FFINHookData& HookData = Data.FindOrAdd(object);
	UClass* clazz = object->GetClass();
	while (clazz) {
		TSet<TSubclassOf<UFINHook>>* hookClasses = HookRegistry.Find(clazz);
		if (hookClasses) for (TSubclassOf<UFINHook> hookClass : *hookClasses) {
			UFINHook* hook = NewObject<UFINHook>(this, hookClass);
			HookData.Hooks.Add(hook);
			hook->Register(object);
		}
		
		if (clazz == UObject::StaticClass()) clazz = nullptr;
		else clazz = clazz->GetSuperClass();
	}
	for (UFINHook* hook : HookData.Hooks) {
		hook->Register(object);
	}
}

void AFINHookSubsystem::ClearHooks(UObject* object) {
	FFINHookData* data = Data.Find(object);
	if (!data) return;
	for (UFINHook* hook : data->Hooks) {
		if (hook) hook->Unregister();
	}
	data->Hooks.Empty();
}

void AFINHookSubsystem::AddListener(UObject* sender, FFINNetworkTrace listener) {
	AttachHooks(sender);
	Data[sender].Listeners.Add(listener);
}

void AFINHookSubsystem::RemoveListener(UObject* sender, UObject* listener) {
	TSet<FFINNetworkTrace>& traces = Data[sender].Listeners;
	traces.Remove(FFINNetworkTrace(listener));
	if (traces.Num() < 1) ClearHooks(sender);
}

