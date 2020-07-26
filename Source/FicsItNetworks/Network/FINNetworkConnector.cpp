#include "FINNetworkConnector.h"

UFINNetworkConnector::UFINNetworkConnector() {}

UFINNetworkConnector::~UFINNetworkConnector() {}

void UFINNetworkConnector::BeginPlay() {
	if (bAddOuterToMerged) AddMerged(GetOuter());

	if (!bIdCreated) {
		ID = FGuid::NewGuid();
		bIdCreated = true;
	}

	// setup circuit
	if (!Circuit) {
		Circuit = NewObject<UFINNetworkCircuit>();
		Circuit->recalculate(this);
	}
}

bool UFINNetworkConnector::ShouldSave_Implementation() const {
	return true;
}

void UFINNetworkConnector::AddConnection(UFINNetworkConnector* connector) {
	if (Connections.Contains(connector)) return;

	Connections.Add(connector);
	connector->AddConnection(this);

	UFINNetworkCircuit::ConnectNodes(this, connector);
}

void UFINNetworkConnector::RemoveConnection(UFINNetworkConnector* connector) {
	if (!Connections.Contains(connector)) return;

	Connections.Remove(connector);
	connector->Connections.Remove(this);
	
	UFINNetworkCircuit::DisconnectNodes(this, connector);
}

bool UFINNetworkConnector::AddCable(AFINNetworkCable* cable) {
	if (MaxCables >= 0 && MaxCables <= Cables.Num()) return false;
	if (Cables.Contains(cable)) return true;
	
	Cables.Add(cable);

	UFINNetworkConnector* OtherConnector = (cable->Connector1 == this) ? ((cable->Connector2 == this) ? nullptr : cable->Connector2) : cable->Connector1;
	OtherConnector->AddCable(cable);

	UFINNetworkCircuit::ConnectNodes(this, OtherConnector);

	return true;
}

void UFINNetworkConnector::RemoveCable(AFINNetworkCable* cable) {
	if (!Cables.Contains(cable)) return;

	if (Cables.Remove(cable) > 0) {
		UFINNetworkConnector* OtherConnector = (cable->Connector1 == this) ? cable->Connector2 : cable->Connector1;
		OtherConnector->Cables.Remove(cable);

		UFINNetworkCircuit::DisconnectNodes(this, OtherConnector);
	}
}

void UFINNetworkConnector::AddComponent(UObject* comp) {
	check(comp->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass()));
	Components.Add(comp);
	UFINNetworkCircuit::ConnectNodes(this, comp);
}

void UFINNetworkConnector::RemoveComponent(UObject* comp) {
	if (Components.Remove(comp) <= 0) return;
	UFINNetworkCircuit::DisconnectNodes(this, comp);
}

bool UFINNetworkConnector::IsConnected(UFINNetworkConnector* con) const {
	if (Connections.Contains(con)) return true;
	for (auto c : Cables) {
		if (c->Connector1 == con || c->Connector2 == con) return true;
	}
	return false;
}

void UFINNetworkConnector::AddMerged(UObject* mergedObj) {
	Merged.Add(mergedObj);
}

void UFINNetworkConnector::RemoveMerged(UObject* mergedObj) {
	Merged.Remove(mergedObj);
}

FGuid UFINNetworkConnector::GetID_Implementation() const {
	return ID;
}

FString UFINNetworkConnector::GetNick_Implementation() const {
	return Nick;
}

void UFINNetworkConnector::SetNick_Implementation(const FString& nick) {
	Nick = nick;
}

bool UFINNetworkConnector::HasNick_Implementation(const FString& nick) {
	return HasNickByNick(nick, Execute_GetNick(this));
}

TSet<UObject*> UFINNetworkConnector::GetMerged_Implementation() const {
	return Merged;
}

TSet<UObject*> UFINNetworkConnector::GetConnected_Implementation() const {
	TSet<UObject*> arr;
	for (auto c : Connections) {
		if (IsValid(c)) arr.Add(c);
	}
	for (auto c : Cables) {
		if (IsValid(c->Connector1)) arr.Add(c->Connector1);
		if (IsValid(c->Connector2)) arr.Add(c->Connector2);
	}
	for (auto c : Components) {
		if (IsValid(c)) arr.Add(c);
	}
	return arr;
}

FFINNetworkTrace UFINNetworkConnector::FindComponent_Implementation(FGuid guid) const {
	UFINNetworkCircuit* circuit = Execute_GetCircuit(this);
	return FFINNetworkTrace((UObject*)this) / circuit->FindComponent(guid);
}

UFINNetworkCircuit * UFINNetworkConnector::GetCircuit_Implementation() const {
	return Circuit;
}

void UFINNetworkConnector::SetCircuit_Implementation(UFINNetworkCircuit * circuit) {
	Circuit = circuit;
}

void UFINNetworkConnector::NotifyNetworkUpdate_Implementation(int type, const TSet<UObject*>& nodes) {
	if (Listeners.Num() < 1) return;
	for (auto node : nodes) {
		auto comp = Cast<IFINNetworkComponent>(node);
		netSig_NetworkUpdate(type, comp->Execute_GetID(node).ToString());
	}
}

void UFINNetworkConnector::AddListener_Implementation(FFINNetworkTrace listener) {
	if (Listeners.Contains(listener)) return;
	Listeners.Add(listener);
}

void UFINNetworkConnector::RemoveListener_Implementation(FFINNetworkTrace listener) {
	Listeners.Remove(listener);
}

TSet<FFINNetworkTrace> UFINNetworkConnector::GetListeners_Implementation() {
	return Listeners;
}

UObject* UFINNetworkConnector::GetSignalSenderOverride_Implementation() {
	return this;
}

void UFINNetworkConnector::HandleSignal(TSharedPtr<FFINSignal> signal, FFINNetworkTrace sender) {
	FFINDynamicStructHolder Holder = FFINDynamicStructHolder::Copy(signal->GetStruct(), signal.Get());
	OnNetworkSignal.Broadcast(Holder, sender);
}

void UFINNetworkConnector::netSig_NetworkUpdate_Implementation(int type, const FString& id) {}
