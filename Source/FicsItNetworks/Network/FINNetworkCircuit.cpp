#include "FINNetworkCircuit.h"

#include "FINNetworkComponent.h"

void UFINNetworkCircuit::addNodeRecursive(TSet<UObject*>& added, UObject* add) {
	auto comp = Cast<IFINNetworkComponent>(add);
	if (!comp) return;

	if (!added.Contains(add)) {
		added.Add(add);
		Nodes.Add(add);
		comp->Execute_SetCircuit(add, this);
		auto connectors = comp->Execute_GetConnected(add);
		for (auto connector : connectors) {
			addNodeRecursive(added, connector);
		}
	}
}

UFINNetworkCircuit::UFINNetworkCircuit() {}

UFINNetworkCircuit::~UFINNetworkCircuit() {}

#pragma optimize("" ,off)
UFINNetworkCircuit* UFINNetworkCircuit::operator+(UFINNetworkCircuit* circuit) {
	if (this == circuit || !IsValid(circuit)) return this;

	UFINNetworkCircuit* from = circuit;
	UFINNetworkCircuit* to = this;

	if (circuit->Nodes.Num() > Nodes.Num()) {
		from = this;
		to = circuit;
	}

	volatile auto fromv = from;
	volatile auto tov = to;

	TSet<UObject*> nodes;
	for (auto& node : to->Nodes) nodes.Add(node.Get());
	for (auto& node : from->Nodes) {
		UObject* obj = node.Get();
		auto comp = Cast<IFINNetworkComponent>(obj);
		if (!comp) continue;
		comp->Execute_SetCircuit(obj, to);
		comp->Execute_NotifyNetworkUpdate(obj, 0, nodes);
	}

	nodes.Empty();
	for (auto& node : from->Nodes) nodes.Add(node.Get());
	for (auto& node : to->Nodes) {
		UObject* obj = node.Get();
		auto comp = Cast<IFINNetworkComponent>(obj);
		comp->Execute_NotifyNetworkUpdate(obj, 0, nodes);
	}

	to->Nodes.Append(from->Nodes);

	return to;
}
#pragma optimize("" ,on)

void UFINNetworkCircuit::recalculate(UObject * component) {
	Nodes.Empty();

	TSet<UObject*> added;
	addNodeRecursive(added, component);
}

bool UFINNetworkCircuit::HasNode(UObject* node) {
	return Nodes.Find(node);
}

UObject* UFINNetworkCircuit::FindComponent(FGuid addr) {
	for (auto node : Nodes) {
		UObject* obj = node.Get();
		auto comp = Cast<IFINNetworkComponent>(obj);

		if (comp && comp->Execute_GetID(obj) == addr) {
			return obj;
		}
	}

	return nullptr;
}

TSet<UObject*> UFINNetworkCircuit::FindComponentsByNick(FString nick) {
	TSet<UObject*> comps;
	for (auto node : Nodes) {
		UObject* obj = node.Get();
		auto comp = Cast<IFINNetworkComponent>(obj);
		if (comp->Execute_HasNick(obj, nick)) comps.Add(obj);
	}

	return comps;
}

TSet<UObject*> UFINNetworkCircuit::GetComponents() {
	TSet<UObject*> out_components;
	for (auto& node : Nodes) out_components.Add(node.Get());
	return out_components;
}
