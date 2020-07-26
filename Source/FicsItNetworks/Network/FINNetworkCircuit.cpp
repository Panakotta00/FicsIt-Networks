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
		IFINNetworkComponent::Execute_SetCircuit(obj, to);
		IFINNetworkComponent::Execute_NotifyNetworkUpdate(obj, 0, nodes);
	}

	nodes.Empty();
	for (auto& node : from->Nodes) nodes.Add(node.Get());
	for (auto& node : to->Nodes) {
		UObject* obj = node.Get();
		auto comp = Cast<IFINNetworkComponent>(obj);
		if (!comp) continue;
		SML::Logging::error("Name: ", TCHAR_TO_UTF8(*obj->GetName()));
		IFINNetworkComponent::Execute_NotifyNetworkUpdate(obj, 0, nodes);
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

bool UFINNetworkCircuit::IsNodeConnected(UObject* start, UObject* node) {
	TSet<UObject*> Searched;
	return IsNodeConnected_Internal(start, node, Searched);
}

void UFINNetworkCircuit::DisconnectNodes(UObject* A, UObject* B) {
	if (!IsNodeConnected(A, B)) {
		UFINNetworkCircuit* CircuitA = NewObject<UFINNetworkCircuit>();
		IFINNetworkComponent::Execute_SetCircuit(A, CircuitA);
		CircuitA->recalculate(A);

		UFINNetworkCircuit* CircuitB = IFINNetworkComponent::Execute_GetCircuit(B);
		
		TSet<UObject*> NodesA = CircuitA->GetComponents();
		TSet<UObject*> NodesB = CircuitB->GetComponents();

		for (const FWeakObjectPtr& Node : CircuitB->Nodes) {
			if (CircuitB->Nodes.Contains(Node)) continue;
			UObject* Obj = Node.Get();
			if (Obj->GetClass()->ImplementsInterface(UFINNetworkComponent::StaticClass())) IFINNetworkComponent::Execute_NotifyNetworkUpdate(Obj, 1, NodesA);
		}
		CircuitB->recalculate(B);
		for (const FWeakObjectPtr& Node : CircuitA->Nodes) {
			if (CircuitA->Nodes.Contains(Node)) continue;
			UObject* obj = Node.Get();
			auto comp = Cast<IFINNetworkComponent>(obj);
			if (comp) comp->Execute_NotifyNetworkUpdate(obj, 1, NodesB);
		}
	}
}

void UFINNetworkCircuit::ConnectNodes(UObject* A, UObject* B) {
	UFINNetworkCircuit* CircuitA = IFINNetworkComponent::Execute_GetCircuit(A);
	UFINNetworkCircuit* CircuitB = IFINNetworkComponent::Execute_GetCircuit(B);
	if (!CircuitB) {
		CircuitB = NewObject<UFINNetworkCircuit>();
		IFINNetworkComponent::Execute_SetCircuit(B, CircuitB);
		CircuitB->recalculate(B);
	}
	if (CircuitA) {
		if (CircuitA != CircuitB) {
			IFINNetworkComponent::Execute_SetCircuit(A, CircuitA = *CircuitA + CircuitB);
			IFINNetworkComponent::Execute_SetCircuit(B, CircuitA);
		}
	} else {
		IFINNetworkComponent::Execute_SetCircuit(A, CircuitB);
		CircuitB->recalculate(A);
	}
}

bool UFINNetworkCircuit::IsNodeConnected_Internal(UObject* self, UObject* node, TSet<UObject*>& Searched) {
	if (Searched.Contains(self)) return false;
	Searched.Add(self);

	if (self == node) return true;
	
	for (UObject* con : IFINNetworkComponent::Execute_GetConnected(self)) {
		if (IsNodeConnected_Internal(con, node, Searched)) return true;
	}

	return false;
}
