#include "FINNetworkConnector.h"

bool UFINNetworkConnector::searchFor(TSet<const UFINNetworkConnector*>& searched, UFINNetworkConnector* connector) const {
	if (searched.Contains(this)) return false;
	searched.Add(this);

	if (this == connector) return true;
	
	for (auto con : Connections) {
		if (con->searchFor(searched, connector)) return true;
	}

	for (auto cable : Cables) {
		auto c1 = cable->Connector1;
		auto c2 = cable->Connector2;
		if (c1->searchFor(searched, connector) || c2->searchFor(searched, connector)) return true;
	}

	return false;
}

void UFINNetworkConnector::removeConnector(UFINNetworkConnector * connector) {
	TSet<const UFINNetworkConnector*> searched;
	if (!searchFor(searched, connector)) {
		connector->Circuit = NewObject<UFINNetworkCircuit>(this);
		connector->Circuit->recalculate(connector);
		
		TSet<UObject*> nodes1 = connector->Circuit->GetComponents();
		TSet<UObject*> nodes2 = Circuit->GetComponents();
		for (auto n : Circuit->Nodes) {
			if (connector->Circuit->Nodes.Contains(n)) continue;
			UObject* obj = n.Get();
			auto comp = Cast<IFINNetworkComponent>(obj);
			comp->Execute_NotifyNetworkUpdate(obj, 1, nodes1);
		}
		Circuit->recalculate(this);
		for (auto n : connector->Circuit->Nodes) {
			if (Circuit->Nodes.Contains(n)) continue;
			UObject* obj = n.Get();
			auto comp = Cast<IFINNetworkComponent>(obj);
			comp->Execute_NotifyNetworkUpdate(obj, 1, nodes2);
		}
	}
}

UFINNetworkConnector::UFINNetworkConnector() {
	Merged.Add(Cast<UObject>(GetOwner()));
}

UFINNetworkConnector::~UFINNetworkConnector() {}

void UFINNetworkConnector::BeginPlay() {
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

	Circuit = connector->Circuit = (Circuit) ? *Circuit + connector->Circuit : connector->Circuit;
}

void UFINNetworkConnector::RemoveConnection(UFINNetworkConnector* connector) {
	if (!Connections.Contains(connector)) return;

	Connections.Remove(connector);
	connector->Connections.Remove(this);
	removeConnector(connector);
}

bool UFINNetworkConnector::AddCable(AFINNetworkCable* cable) {
	if (MaxCables >= 0 && MaxCables <= Cables.Num()) return false;

	Cables.Add(cable);

	auto c = (cable->Connector1 == this) ? ((cable->Connector2 == this) ? nullptr : cable->Connector2) : cable->Connector1;

	if (c && c->Circuit) {
		if (this->Circuit) {
			Circuit = c->Circuit = *cable->Connector1->Circuit + cable->Connector2->Circuit;
		} else {
			Circuit = c->Circuit;
			Circuit->recalculate(this);
		}
	}

	return true;
}

void UFINNetworkConnector::RemoveCable(AFINNetworkCable* cable) {
	if (!Cables.Contains(cable)) return;

	Cables.Remove(cable);

	auto other = (cable->Connector1 == this) ? cable->Connector2 : cable->Connector1;
	other->Cables.Remove(cable);

	removeConnector(other);
}

bool UFINNetworkConnector::IsConnected(UFINNetworkConnector* con) const {
	if (Connections.Contains(con)) return true;
	for (auto c : Cables) {
		if (c->Connector1 == con || c->Connector2 == con) return true;
	}
	return false;
}

bool UFINNetworkConnector::SearchFor(UFINNetworkConnector * conn) const {
	TSet<const UFINNetworkConnector*> searched;
	return searchFor(searched, conn);
}

void UFINNetworkConnector::AddMerged(UObject* mergedObj) {
	Merged.Add(mergedObj);
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
	return IFINNetworkComponent::FindComponentByCircuit(guid, Execute_GetCircuit(this));
}

UFINNetworkCircuit * UFINNetworkConnector::GetCircuit_Implementation() const {
	return Circuit;
}

void UFINNetworkConnector::SetCircuit_Implementation(UFINNetworkCircuit * circuit) {
	Circuit = circuit;
}

void UFINNetworkConnector::NotifyNetworkUpdate_Implementation(int type, const TSet<UObject*>& nodes) {
	if (Listeners.Num() < 1) return;
	auto func = FindFunction(L"luaSig_NetworkUpdate");
	for (auto node : nodes) {
		struct {
			int32 t;
			FString n;
		} params;
		params.t = type;
		auto comp = Cast<IFINNetworkComponent>(node);
		params.n = comp->Execute_GetID(node).ToString();
		ProcessEvent(func, &params);
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

void UFINNetworkConnector::netSig_NetworkUpdate(int type, FString id) {}
