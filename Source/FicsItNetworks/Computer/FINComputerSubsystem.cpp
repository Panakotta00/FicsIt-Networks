#include "FINComputerSubsystem.h"

#include "FicsItNetworks/FINSubsystemHolder.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPowerCircuit.h"
#include "util/Logging.h"

bool FFINFactoryHook::Serialize(FArchive& Ar) {
	Ar << CountOfReferences;
	Ar << CurrentSample;
	Ar << Listeners;
	Ar << Samples;

	return true;
}

void FFINFactoryHook::Update() {
	Samples.Add(CurrentSample);
	CurrentSample = 0;
	while (Samples.Num() > 60) {
		Samples.RemoveAt(0);
	}
}

int32 FFINFactoryHook::GetSampleSum() {
	int32 Sum = 0;
	for (int32 Sample : Samples) {
		Sum += Sample;
	}
	return Sum;
}

FArchive& operator<<(FArchive& Ar, FFINFactoryHook& Hook) {
	Hook.Serialize(Ar);
	return Ar;
}

void AFINComputerSubsystem::Serialize(FArchive& Ar) {
	int32 FactoryHookCount = FactoryHooks.Num();
	Ar << FactoryHookCount;
	if (Ar.IsSaving()) for (TPair<TWeakObjectPtr<UFGFactoryConnectionComponent>, FFINFactoryHook>& hook : FactoryHooks) {
		UObject* ptr = hook.Key.Get();
		Ar << ptr;
		Ar << hook.Value;
	} else if (Ar.IsLoading()) {
		FactoryHooks.Empty();
		for (int i = 0; i < FactoryHookCount; ++i) {
			TPair<TWeakObjectPtr<UFGFactoryConnectionComponent>, FFINFactoryHook> hook;
			UObject* ptr = nullptr;
			Ar << ptr;
			hook.Key = Cast<UFGFactoryConnectionComponent>(ptr);
			Ar << hook.Value;
			FactoryHooks.Add(hook);
		}
	}
	int64 count = PowerCircuitListeners.Num();
	Ar << count;
	if (Ar.IsSaving()) for (TPair<TWeakObjectPtr<UFGPowerCircuit>, TSet<FFINNetworkTrace>>& pair : PowerCircuitListeners) {
		UObject* ptr = pair.Key.Get();
		Ar << ptr;
		Ar << pair.Value;
	} else if (Ar.IsLoading()) {
		PowerCircuitListeners.Empty();
		for (int i = 0; i < count; ++i) {
			TPair<TWeakObjectPtr<UFGPowerCircuit>, TSet<FFINNetworkTrace>> pair;
			UObject* ptr = nullptr;
			Ar << ptr;
			pair.Key = Cast<UFGPowerCircuit>(ptr);
			Ar << pair.Value;
			PowerCircuitListeners.Add(pair);
		}
	}
}

void AFINComputerSubsystem::BeginPlay() {
	UWorld* world = GetWorld();
	if (world) world->GetTimerManager().SetTimer(FactoryHookTimer, this, &AFINComputerSubsystem::UpdateHooks, 1.0, true, 0.0f);
}

void AFINComputerSubsystem::EndPlay(EEndPlayReason::Type Reson) {
	UWorld* world = GetWorld();
	if (world) world->GetTimerManager().ClearTimer(FactoryHookTimer);
}

bool AFINComputerSubsystem::ShouldSave_Implementation() const {
	return true;
}

AFINComputerSubsystem* AFINComputerSubsystem::GetComputerSubsystem(UObject* WorldContext) {
	return GetSubsystemHolder<UFINSubsystemHolder>(WorldContext)->ComputerSubsystem;
}

void AFINComputerSubsystem::UpdateHooks() {
	MutexFactoryHooks.Lock();
	for (TPair<TWeakObjectPtr<UFGFactoryConnectionComponent>, FFINFactoryHook>& hook : FactoryHooks) {
		hook.Value.Update();
	}
	MutexFactoryHooks.Unlock();
}

void AFINComputerSubsystem::RemoveHook(FFINNetworkTrace Connector) {
	UFGFactoryConnectionComponent* FactoryConnector = Cast<UFGFactoryConnectionComponent>(*Connector);
	FFINFactoryHook* hook = FactoryHooks.Find(FactoryConnector);
	if (!hook) return;
	--hook->CountOfReferences;
	if (hook->CountOfReferences < 1) {
		FactoryHooks.Remove(FactoryConnector);
	}
}
