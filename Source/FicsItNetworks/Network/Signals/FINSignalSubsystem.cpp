#include "FINSignalSubsystem.h"


#include "FINSignalListener.h"
#include "FINSubsystemHolder.h"
#include "mod/ModSubsystems.h"

bool AFINSignalSubsystem::ShouldSave_Implementation() const {
	return true;
}

AFINSignalSubsystem* AFINSignalSubsystem::GetSignalSubsystem(UObject* WorldContext) {
	return GetSubsystemHolder<UFINSubsystemHolder>(WorldContext)->SignalSubsystem;
}

void AFINSignalSubsystem::BroadcastSignal(UObject* Sender, TFINDynamicStruct<FFINSignal> Signal) {
	FFINSignalListeners* ListenerList = Listeners.Find(Sender);
	if (!ListenerList) return;
	for (const FFINNetworkTrace& ReceiverTrace : ListenerList->Listeners) {
		IFINSignalListener* Receiver = Cast<IFINSignalListener>(ReceiverTrace.Get());
		if (Receiver) {
			Receiver->HandleSignal(Signal, ReceiverTrace.Reverse());
		}
	}
}

// TODO: Make sure that using network traces work in combination with TArray::AddUnique, TArray::RemoveAll, etc.

void AFINSignalSubsystem::Listen(UObject* Sender, const FFINNetworkTrace& Receiver) {
	TArray<FFINNetworkTrace>& ListenerList = Listeners.FindOrAdd(Sender).Listeners;
	ListenerList.AddUnique(Receiver);
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
}

void AFINSignalSubsystem::IgnoreAll(UObject* Receiver) {
	TArray<UObject*> Senders;
	Listeners.GetKeys(Senders);
	for (UObject* Sender : Senders) {
		Ignore(Sender, Receiver);
	}
}
