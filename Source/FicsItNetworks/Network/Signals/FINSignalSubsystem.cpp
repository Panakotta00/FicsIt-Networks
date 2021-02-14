#include "FINSignalSubsystem.h"

#include "FicsItNetworksModule.h"
#include "FINSignalListener.h"
#include "FINSubsystemHolder.h"
#include "mod/ModSubsystems.h"

bool AFINSignalSubsystem::ShouldSave_Implementation() const {
	return true;
}

void AFINSignalSubsystem::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	AFINHookSubsystem* HookSubsystem = AFINHookSubsystem::GetHookSubsystem(this);
	if (HookSubsystem) for (const TPair<UObject*, FFINSignalListeners>& Sender : Listeners) {
		HookSubsystem->AttachHooks(Sender.Key);
	} else {
		UE_LOG(LogFicsItNetworks, Warning, TEXT("Hook Subsystem not found! Unable to reattach hooks!"));
	}
}

void AFINSignalSubsystem::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(AFINHookSubsystem::GetHookSubsystem(this));
}

AFINSignalSubsystem* AFINSignalSubsystem::GetSignalSubsystem(UObject* WorldContext) {
	return GetSubsystemHolder<UFINSubsystemHolder>(WorldContext)->SignalSubsystem;
}

void AFINSignalSubsystem::BroadcastSignal(UObject* Sender, const FFINSignalData& Signal) {
	FFINSignalListeners* ListenerList = Listeners.Find(Sender);
	if (!ListenerList) return;
	for (const FFINNetworkTrace& ReceiverTrace : ListenerList->Listeners) {
		if (&ReceiverTrace == nullptr) {
			UE_LOG(LogFicsItNetworks, Warning, TEXT("SignalSubsystem: Invalid receiver trave. Sender: %s, ListenerList: %p, Listeners.Num(): %i"), *Sender->GetName(), ListenerList, ListenerList->Listeners.Num());
			continue;
		}
		IFINSignalListener* Receiver = Cast<IFINSignalListener>(ReceiverTrace.Get());
		if (Receiver) {
			Receiver->HandleSignal(Signal, ReceiverTrace.Reverse());
		}
	}
}

void AFINSignalSubsystem::Listen(UObject* Sender, const FFINNetworkTrace& Receiver) {
	TArray<FFINNetworkTrace>& ListenerList = Listeners.FindOrAdd(Sender).Listeners;
	ListenerList.AddUnique(Receiver);
	AFINHookSubsystem::GetHookSubsystem(Sender)->AttachHooks(Sender);
}

void AFINSignalSubsystem::Ignore(UObject* Sender, UObject* Receiver) {
	FFINSignalListeners* ListenerList = Listeners.Find(Sender);
	if (!ListenerList) return;
	for (int i = 0; i < ListenerList->Listeners.Num(); ++i) {
		if (ListenerList->Listeners[i].GetUnderlyingPtr() == Receiver) {
			ListenerList->Listeners.RemoveAt(i);
			--i;
		}
	}
	if (ListenerList->Listeners.Num() < 1) AFINHookSubsystem::GetHookSubsystem(Sender)->ClearHooks(Sender);
}

void AFINSignalSubsystem::IgnoreAll(UObject* Receiver) {
	TArray<UObject*> Senders;
	Listeners.GetKeys(Senders);
	for (UObject* Sender : Senders) {
		Ignore(Sender, Receiver);
	}
}

TArray<UObject*> AFINSignalSubsystem::GetListening(UObject* Reciever) {
	TArray<UObject*> Listening;
	for (TPair<UObject*, FFINSignalListeners> Sender : Listeners) {
		if (Sender.Value.Listeners.Contains(FFINNetworkTrace(Reciever))) {
			Listening.Add(Sender.Key);
		}
	}
	return Listening;
}
