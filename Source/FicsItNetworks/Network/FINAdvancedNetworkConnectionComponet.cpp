#include "FINAdvancedNetworkConnectionComponent.h"

#include "FINNetworkCircuit.h"

UFINAdvancedNetworkConnectionComponent::UFINAdvancedNetworkConnectionComponent() {}

UFINAdvancedNetworkConnectionComponent::~UFINAdvancedNetworkConnectionComponent() {}

void UFINAdvancedNetworkConnectionComponent::BeginPlay() {
	if (bAddOuterToMerged) AddMerged(GetOuter());

	if (!bIdCreated) {
		ID = FGuid::NewGuid();
		bIdCreated = true;
	}

	// setup circuit
	if (!Circuit) {
		Circuit = NewObject<UFINNetworkCircuit>();
		Circuit->Recalculate(this);
	}
}

void UFINAdvancedNetworkConnectionComponent::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
}

bool UFINAdvancedNetworkConnectionComponent::ShouldSave_Implementation() const {
	return true;
}

void UFINAdvancedNetworkConnectionComponent::NotifyNetworkUpdate_Implementation(int Type, const TSet<UObject*>& Nodes) {
	if (Listeners.Num() < 1) return;
	for (UObject* Node : Nodes) {
		if (Node->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) {
			netSig_NetworkUpdate(Type, IFINNetworkComponent::Execute_GetID(Node).ToString());
		}
	}
}

void UFINAdvancedNetworkConnectionComponent::AddMerged(UObject* MergedObj) {
	Merged.Add(MergedObj);
}

void UFINAdvancedNetworkConnectionComponent::RemoveMerged(UObject* MergedObj) {
	Merged.Remove(MergedObj);
}

FGuid UFINAdvancedNetworkConnectionComponent::GetID_Implementation() const {
	return ID;
}

FString UFINAdvancedNetworkConnectionComponent::GetNick_Implementation() const {
	return Nick;
}

void UFINAdvancedNetworkConnectionComponent::SetNick_Implementation(const FString& Nick) {
	this->Nick = Nick;
}

bool UFINAdvancedNetworkConnectionComponent::HasNick_Implementation(const FString& Nick) {
	return HasNickByNick(Nick, Execute_GetNick(this));
}

TSet<UObject*> UFINAdvancedNetworkConnectionComponent::GetMerged_Implementation() const {
	return Merged;
}

bool UFINAdvancedNetworkConnectionComponent::AccessPermitted_Implementation(FGuid ID) const {
	return true;
}

void UFINAdvancedNetworkConnectionComponent::AddListener_Implementation(FFINNetworkTrace Listener) {
	if (Listeners.Contains(Listener)) return;
	Listeners.Add(Listener);
}

void UFINAdvancedNetworkConnectionComponent::RemoveListener_Implementation(FFINNetworkTrace Listener) {
	Listeners.Remove(Listener);
}

TSet<FFINNetworkTrace> UFINAdvancedNetworkConnectionComponent::GetListeners_Implementation() {
	return Listeners;
}

UObject* UFINAdvancedNetworkConnectionComponent::GetSignalSenderOverride_Implementation() {
	return this;
}

void UFINAdvancedNetworkConnectionComponent::HandleSignal(TSharedPtr<FFINSignal> Signal, FFINNetworkTrace Sender) {
	FFINDynamicStructHolder Holder = FFINDynamicStructHolder::Copy(Signal->GetStruct(), Signal.Get());
	OnNetworkSignal.Broadcast(Holder, Sender);
}

void UFINAdvancedNetworkConnectionComponent::netSig_NetworkUpdate_Implementation(int type, const FString& id) {}
