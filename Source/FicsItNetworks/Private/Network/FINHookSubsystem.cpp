#include "Network/FINHookSubsystem.h"
#include "Subsystem/SubsystemActorManager.h"

TMap<UClass*, TSet<TSubclassOf<UFINHook>>> AFINHookSubsystem::HookRegistry;

AFINHookSubsystem* AFINHookSubsystem::GetHookSubsystem(UObject* WorldContext) {
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINHookSubsystem>();
}

void AFINHookSubsystem::RegisterHook(UClass* clazz, TSubclassOf<UFINHook> hook) {
	HookRegistry.FindOrAdd(clazz).Add(hook);
}

void AFINHookSubsystem::AttachHooks(UObject* object) {
	if (!IsValid(object)) return;
	FScopeLock Lock(&DataLock);
	if (Data.Contains(object)) return;
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
