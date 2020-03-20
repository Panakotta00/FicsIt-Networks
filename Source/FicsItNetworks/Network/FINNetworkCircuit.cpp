#include "FINNetworkCircuit.h"

#include "FINNetworkComponent.h"

void UFINNetworkCircuit::addNodeRecursive(TSet<UObject*>& added, UObject* add) {
	auto comp = Cast<IFINNetworkComponent>(add);
	if (!comp) return;

	if (!added.Contains(add)) {
		added.Add(add);
		Nodes.Add(add);
		comp->SetCircuit(this);
		auto connectors = comp->GetConnected();
		for (auto connector : connectors) {
			addNodeRecursive(added, connector);
		}
	}
}

UFINNetworkCircuit::UFINNetworkCircuit() {}

UFINNetworkCircuit::~UFINNetworkCircuit() {}

UFINNetworkCircuit* UFINNetworkCircuit::operator+(UFINNetworkCircuit* circuit) {
	if (this == circuit || !circuit) return this;

	UFINNetworkCircuit* from = circuit;
	UFINNetworkCircuit* to = this;

	if (circuit->Nodes.Num() > Nodes.Num()) {
		from = this;
		to = circuit;
	}

	TSet<FFINNetworkTrace> nodes;
	for (auto& node : to->Nodes) nodes.Add(FFINNetworkTrace(node.Get()));
	for (auto& node : from->Nodes) {
		auto comp = Cast<IFINNetworkComponent>(node.Get());
		if (!comp) continue;
		comp->SetCircuit(to);
		comp->NotifyNetworkUpdate(0, nodes);
	}

	nodes.Empty();
	for (auto& node : from->Nodes) nodes.Add(FFINNetworkTrace(node.Get()));
	for (auto& node : to->Nodes) {
		auto comp = Cast<IFINNetworkComponent>(node.Get());
		comp->NotifyNetworkUpdate(0, nodes);
	}

	to->Nodes.Append(from->Nodes);

	return to;
}

void UFINNetworkCircuit::recalculate(UObject * component) {
	Nodes.Empty();

	TSet<UObject*> added;
	addNodeRecursive(added, component);
}

bool UFINNetworkCircuit::HasNode(UObject* node) {
	return Nodes.Find(node);
}

FFINNetworkTrace UFINNetworkCircuit::FindComponent(FGuid addr) {
	for (auto node : Nodes) {
		auto comp = Cast<IFINNetworkComponent>(node.Get());

		if (comp->GetID() == addr) {
			return FFINNetworkTrace(node.Get());
		}
	}

	return FFINNetworkTrace(nullptr);
}

TSet<FFINNetworkTrace> UFINNetworkCircuit::FindComponentsByNick(FString nick) {
	TSet<FFINNetworkTrace> comps;
	for (auto node : Nodes) {
		auto comp = Cast<IFINNetworkComponent>(node.Get());
		if (comp->HasNick(nick)) comps.Add(FFINNetworkTrace(node.Get()));
	}

	return comps;
}

void UFINNetworkCircuit::GetComponents(TSet<FFINNetworkTrace>& out_components) {
	for (auto& node : Nodes) out_components.Add(FFINNetworkTrace(node.Get()));
}
