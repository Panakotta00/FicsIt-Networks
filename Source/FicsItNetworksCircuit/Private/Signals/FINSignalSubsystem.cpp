#include "Signals/FINSignalSubsystem.h"

#include "FicsItNetworksCircuit.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Engine/Engine.h"
#include "FIRHookSubsystem.h"
#include "Signals/FINSignalListener.h"

void FFINSignalListeners::AddStructReferencedObjects(FReferenceCollector& ReferenceCollector) const {
	for (const FFIRTrace& Trace : Listeners) {
		Trace.AddStructReferencedObjects(ReferenceCollector);
	}
}

bool AFINSignalSubsystem::ShouldSave_Implementation() const {
	return true;
}

void AFINSignalSubsystem::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	Cleanup();
}

void AFINSignalSubsystem::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	
}

void AFINSignalSubsystem::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(AFIRHookSubsystem::GetHookSubsystem(this));
}

void AFINSignalSubsystem::BeginPlay() {
	Super::BeginPlay();

	AFIRHookSubsystem* HookSubsystem = AFIRHookSubsystem::GetHookSubsystem(this);
	if (HookSubsystem) for (const TPair<UObject*, FFINSignalListeners>& Sender : Senders) {
		HookSubsystem->AttachHooks(Sender.Key);
	} else {
		UE_LOG(LogFicsItNetworksCircuit, Warning, TEXT("Hook Subsystem not found! Unable to reattach hooks!"));
	}
}

void AFINSignalSubsystem::Cleanup() {
	TArray<UObject*> ListenerKeys;
	Senders.GetKeys(ListenerKeys);
	for (UObject* Sender : ListenerKeys) {
		if (!IsValid(Sender)) {
			Senders.Remove(Sender);
		} else {
			TArray<FFIRTrace>& Listen = Senders[Sender].Listeners;
			for (int i = 0; i < Listen.Num(); ++i) {
				const FFIRTrace& Listener = Listen[i];
				if (!IsValid(Listener.GetUnderlyingPtr())) {
					Listen.RemoveAt(i--);
				}
			}
			if (Listen.Num() < 1) {
				Senders.Remove(Sender);
			}
		}
	}
}

AFINSignalSubsystem* AFINSignalSubsystem::GetSignalSubsystem(UObject* WorldContext) {
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINSignalSubsystem>();
}

void AFINSignalSubsystem::BroadcastSignal(UObject* Sender, const FFINSignalData& Signal) {
	FFINSignalListeners* ListenerList = Senders.Find(Sender);
	if (!ListenerList) return;
	for (const FFIRTrace& ReceiverTrace : ListenerList->Listeners) {
		if (&ReceiverTrace == nullptr) {
			UE_LOG(LogFicsItNetworksCircuit, Warning, TEXT("SignalSubsystem: Invalid receiver trave. Sender: %s, ListenerList: %p, Listeners.Num(): %i"), *Sender->GetName(), ListenerList, ListenerList->Listeners.Num());
			continue;
		}
		IFINSignalListener* Receiver = Cast<IFINSignalListener>(ReceiverTrace.Get());
		if (Receiver) {
			Receiver->HandleSignal(Signal, ReceiverTrace.Reverse());
		}
	}
}

void AFINSignalSubsystem::Listen(UObject* Sender, const FFIRTrace& Receiver) {
	TArray<FFIRTrace>& ListenerList = Senders.FindOrAdd(Sender).Listeners;
	ListenerList.AddUnique(Receiver);
	AFIRHookSubsystem::GetHookSubsystem(Sender)->AttachHooks(Sender);
}

void AFINSignalSubsystem::Ignore(UObject* Sender, UObject* Receiver) {
	FFINSignalListeners* ListenerList = Senders.Find(Sender);
	if (!ListenerList) return;
	for (int i = 0; i < ListenerList->Listeners.Num(); ++i) {
		if (ListenerList->Listeners[i].GetUnderlyingPtr() == Receiver) {
			ListenerList->Listeners.RemoveAt(i);
			--i;
		}
	}
	if (!Sender) return;
	AFIRHookSubsystem* HookSubsystem = AFIRHookSubsystem::GetHookSubsystem(Sender);
	if (ListenerList->Listeners.Num() < 1 && HookSubsystem) HookSubsystem->ClearHooks(Sender);
}

void AFINSignalSubsystem::IgnoreAll(UObject* Receiver) {
	TArray<UObject*> senders;
	Senders.GetKeys(senders);
	for (UObject* Sender : senders) {
		Ignore(Sender, Receiver);
	}
}

TArray<FFIRTrace> AFINSignalSubsystem::GetListening(UObject* Reciever) {
	TArray<FFIRTrace> Listening;
	for (TPair<UObject*, FFINSignalListeners> Sender : Senders) {
		size_t index = Sender.Value.Listeners.Find(FFIRTrace(Reciever));
		if (index != INDEX_NONE) {
			Listening.Add(Sender.Value.Listeners[index].Reverse() / Sender.Key);
		}
	}
	return Listening;
}
