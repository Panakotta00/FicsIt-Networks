#include "FIRHookSubsystem.h"

#include "Engine/Engine.h"
#include "Subsystem/SubsystemActorManager.h"

TMap<UClass*, TSet<TSubclassOf<UFIRHook>>> AFIRHookSubsystem::HookRegistry;

void AFIRHookSubsystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) {
	FScopeLock Lock(&Cast<AFIRHookSubsystem>(InThis)->DataLock);

	Super::AddReferencedObjects(InThis, Collector);
}

void AFIRHookSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	TArray<UObject*> Keys;
	{
		FScopeLock Lock(&DataLock);
		Data.GetKeys(Keys);
	}
	for (UObject* obj : Keys) {
		ClearHooks(obj);
	}
}

AFIRHookSubsystem* AFIRHookSubsystem::GetHookSubsystem(UObject* WorldContext) {
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFIRHookSubsystem>();
}

void AFIRHookSubsystem::RegisterHook(UClass* clazz, TSubclassOf<UFIRHook> hook) {
	HookRegistry.FindOrAdd(clazz).Add(hook);
}

void AFIRHookSubsystem::AttachHooks(UObject* object) {
	if (!IsValid(object)) return;
	FScopeLock Lock(&DataLock);
	ClearHooks(object);
	FFIRHookData& HookData = Data.FindOrAdd(object);
	UClass* clazz = object->GetClass();
	while (clazz) {
		TSet<TSubclassOf<UFIRHook>>* hookClasses = HookRegistry.Find(clazz);
		if (hookClasses) for (TSubclassOf<UFIRHook> hookClass : *hookClasses) {
			UFIRHook* hook = NewObject<UFIRHook>(this, hookClass);
			hook->ClearInternalFlags(EInternalObjectFlags::Async);
			HookData.Hooks.Add(hook);
		}
		
		if (clazz == UObject::StaticClass()) clazz = nullptr;
		else clazz = clazz->GetSuperClass();
	}
	for (UFIRHook* hook : HookData.Hooks) {
		hook->Register(object);
	}
}

void AFIRHookSubsystem::ClearHooks(UObject* object) {
	FScopeLock Lock(&DataLock);
	FFIRHookData data;
	if (!Data.RemoveAndCopyValue(object, data)) return;
	for (UFIRHook* hook : data.Hooks) {
		if (hook) hook->Unregister();
	}
}
