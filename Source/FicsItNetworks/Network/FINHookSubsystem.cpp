#include "FINHookSubsystem.h"

#include "FicsItNetworks/FINSubsystemHolder.h"

TMap<UClass*, TSet<TSubclassOf<UFINHook>>> AFINHookSubsystem::HookRegistry;

AFINHookSubsystem* AFINHookSubsystem::GetHookSubsystem(UObject* WorldContext) {
	return UModSubsystemHolder::GetSubsystemHolder<UFINSubsystemHolder>(WorldContext)->HookSubsystem;
}

void AFINHookSubsystem::RegisterHook(UClass* clazz, TSubclassOf<UFINHook> hook) {
	HookRegistry.FindOrAdd(clazz).Add(hook);
}

void AFINHookSubsystem::AttachHooks(UObject* object) {
	if (!IsValid(object)) return;
	FScopeLock Lock(&DataLock);
	ClearHooks(object);
	FFINHookData& HookData = Data.FindOrAdd(object);
	UClass* clazz = object->GetClass();
	while (clazz) {
		TSet<TSubclassOf<UFINHook>>* hookClasses = HookRegistry.Find(clazz);
		if (hookClasses) for (TSubclassOf<UFINHook> hookClass : *hookClasses) {
			UFINHook* hook = NewObject<UFINHook>(this, hookClass);
			HookData.Hooks.Add(hook);
		}
		
		if (clazz == UObject::StaticClass()) clazz = nullptr;
		else clazz = clazz->GetSuperClass();
	}
	for (UFINHook* hook : HookData.Hooks) {
		hook->Register(object);
	}
}

void AFINHookSubsystem::ClearHooks(UObject* object) {
	FScopeLock Lock(&DataLock);
	FFINHookData* data = Data.Find(object);
	if (!data) return;
	for (UFINHook* hook : data->Hooks) {
		if (hook) hook->Unregister();
	}
	data->Hooks.Empty();
}
