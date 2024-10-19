#include "FicsItKernel/Network/NetworkController.h"

#include "FicsItNetworksCustomVersion.h"
#include "FINComputerSubsystem.h"
#include "FINNetworkCircuit.h"
#include "FINNetworkCircuitNode.h"
#include "FINNetworkUtils.h"

void UFINKernelNetworkController::Serialize(FStructuredArchive::FRecord Record) {
	Super::Serialize(Record);

	if (!Record.GetUnderlyingArchive().IsSaveGame()) return;
	
	if (AFINComputerSubsystem::GetComputerSubsystem(this)->Version < FINKernelRefactor) return;
	
	// serialize signals
	int32 SignalCount = SignalQueue.Num();
	FStructuredArchive::FArray SignalListRecord = Record.EnterArray(SA_FIELD_NAME(TEXT("Signals")), SignalCount);
	for (int i = 0; i < SignalCount; ++i) {
		FStructuredArchive::FRecord SignalRecord = SignalListRecord.EnterElement().EnterRecord();
		
		FFINSignalData Signal;
		FFIRTrace Trace;
		if (SignalRecord.GetUnderlyingArchive().IsSaving()) {
			const TTuple<FFINSignalData, FFIRTrace>& SignalData = SignalQueue[i];
			Signal = SignalData.Key;
			Trace = SignalData.Value;
		}

		SignalRecord.EnterField(SA_FIELD_NAME(TEXT("Signal"))) << Signal;
		SignalRecord.EnterField(SA_FIELD_NAME(TEXT("Trace"))) << Trace;
		
		if (SignalRecord.GetUnderlyingArchive().IsLoading()) {
			SignalQueue.Add(TPair<FFINSignalData, FFIRTrace>{Signal, Trace});
		}
	}
}

void UFINKernelNetworkController::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	bLockSignalReceiving = true;
}

void UFINKernelNetworkController::PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	bLockSignalReceiving = false;
}

void UFINKernelNetworkController::PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	bLockSignalReceiving = true;
}

void UFINKernelNetworkController::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	bLockSignalReceiving = false;
}

void UFINKernelNetworkController::SetComponent(TScriptInterface<IFINNetworkComponent> InNetworkComponent) {
	Component = InNetworkComponent;
}

TScriptInterface<IFINNetworkComponent> UFINKernelNetworkController::GetComponent() const {
	return Component;
}

void UFINKernelNetworkController::HandleSignal(const FFINSignalData& signal, const FFIRTrace& sender) {
	PushSignal(signal, sender);
}

FFINSignalData UFINKernelNetworkController::PopSignal(FFIRTrace& OutSender) {
	if (GetSignalCount() < 1) return FFINSignalData();
	FScopeLock Lock(&MutexSignals);
	const TTuple<FFINSignalData, FFIRTrace> Signal = SignalQueue[0];
	SignalQueue.RemoveAt(0);
	OutSender = Signal.Value;
	return Signal.Key;
}

void UFINKernelNetworkController::PushSignal(const FFINSignalData& signal, const FFIRTrace& sender) {
	FScopeLock Lock(&MutexSignals);
	if (GetSignalCount() >= MaxSignalCount || bLockSignalReceiving) return;
	SignalQueue.Add(TPair<FFINSignalData, FFIRTrace>{signal, sender});
}

void UFINKernelNetworkController::ClearSignals() {
	FScopeLock Lock(&MutexSignals);
	SignalQueue.Empty();
}

size_t UFINKernelNetworkController::GetSignalCount() {
	FScopeLock Lock(&MutexSignals);
	return SignalQueue.Num();
}

FFIRTrace UFINKernelNetworkController::GetComponentByID(const FGuid& InID) const {
	if (!Component.GetObject()->Implements<UFINNetworkCircuitNode>()) return FFIRTrace();
	return FFIRTrace(Component.GetObject()) / IFINNetworkCircuitNode::Execute_GetCircuit(Component.GetObject())->FindComponent(InID, Component).GetObject();
}

TSet<FFIRTrace> UFINKernelNetworkController::GetComponentByNick(const FString& InNick) const {
	if (!Component.GetObject()->Implements<UFINNetworkCircuitNode>()) return TSet<FFIRTrace>();
	TSet<FFIRTrace> OutComponents;
	TSet<UObject*> Components = IFINNetworkCircuitNode::Execute_GetCircuit(Component.GetObject())->FindComponentsByNick(InNick, Component);
	for (UObject* Comp : Components) {
		OutComponents.Add(FFIRTrace(Component.GetObject()) / Comp);
	}
	return OutComponents;
}

TSet<FFIRTrace> UFINKernelNetworkController::GetComponentByClass(UClass* InClass, bool bRedirect) const {
	if (!IsValid(InClass) || !Component.GetObject()->Implements<UFINNetworkCircuitNode>()) return TSet<FFIRTrace>();
	TSet<FFIRTrace> outComps;
	TSet<UObject*> Comps = IFINNetworkCircuitNode::Execute_GetCircuit(Component.GetObject())->GetComponents();
	for (UObject* Comp : Comps) {
		if (bRedirect) {
			UObject* RedirectedComp = UFINNetworkUtils::RedirectIfPossible(FFIRTrace(Comp)).Get();
			if (!RedirectedComp->IsA(InClass)) continue;
		} else if (!Comp->IsA(InClass)) continue;
		if (!IFINNetworkComponent::Execute_AccessPermitted(Comp, IFINNetworkComponent::Execute_GetID(Component.GetObject()))) continue;
		outComps.Add(FFIRTrace(Component.GetObject()) / Comp);
	}
	return outComps;
}
