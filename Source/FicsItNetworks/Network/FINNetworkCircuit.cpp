#include "FINNetworkCircuit.h"

#include "FINNetworkComponent.h"

void UFINNetworkCircuit::AddNodeRecursive(TSet<TScriptInterface<IFINNetworkCircuitNode>>& Added, TScriptInterface<IFINNetworkCircuitNode> Add) {
	if (Add.GetObject() && !Added.Contains(Add)) {
		Added.Add(Add);
		Nodes.Add(Add.GetObject());
		IFINNetworkCircuitNode::Execute_SetCircuit(Add.GetObject(), this);
		TSet<UObject*> Nodes = IFINNetworkCircuitNode::Execute_GetConnected(Add.GetObject());
		for (UObject* Node : Nodes) {
			AddNodeRecursive(Added, Node);
		}
	}
}

UFINNetworkCircuit::UFINNetworkCircuit() {}

UFINNetworkCircuit::~UFINNetworkCircuit() {}

UFINNetworkCircuit* UFINNetworkCircuit::operator+(UFINNetworkCircuit* Circuit) {
	if (this == Circuit || !IsValid(Circuit)) return this;

	UFINNetworkCircuit* From = Circuit;
	UFINNetworkCircuit* To = this;

	if (Circuit->Nodes.Num() > Nodes.Num()) {
		From = this;
		To = Circuit;
	}

	TSet<UObject*> ToNodes;
	for (const TSoftObjectPtr<UObject>& ToNode : To->Nodes) {
		UObject* O = ToNode.Get();
		if (O) ToNodes.Add(O);
	}
	for (const TSoftObjectPtr<UObject>& Node : From->Nodes) {
		UObject* Obj = Node.Get();
		IFINNetworkCircuitNode::Execute_SetCircuit(Obj, To);
		IFINNetworkCircuitNode::Execute_NotifyNetworkUpdate(Obj, 0, ToNodes);
	}

	TSet<UObject*> FromNodes;
	for (const TSoftObjectPtr<UObject>& FromNode : From->Nodes) {
		UObject* O = FromNode.Get();
		if (O) FromNodes.Add(O);
	}
	for (const TSoftObjectPtr<UObject>& Node : To->Nodes) {
		UObject* Obj = Node.Get();
		IFINNetworkCircuitNode::Execute_NotifyNetworkUpdate(Obj, 0, FromNodes);
	}

	To->Nodes.Append(From->Nodes);

	return To;
}

void UFINNetworkCircuit::Recalculate(const TScriptInterface<IFINNetworkCircuitNode>& Node) {
	Nodes.Empty();

	TSet<TScriptInterface<IFINNetworkCircuitNode>> Added;
	AddNodeRecursive(Added, Node);
}

bool UFINNetworkCircuit::HasNode(const TScriptInterface<IFINNetworkCircuitNode>& Node) {
	return Nodes.Find(Node.GetObject());
}

TScriptInterface<IFINNetworkComponent> UFINNetworkCircuit::FindComponent(const FGuid& ID, const TScriptInterface<IFINNetworkComponent>& Requester) {
	FGuid ReqID = (Requester) ? IFINNetworkComponent::Execute_GetID(Requester.GetObject()) : FGuid();
	for (const TSoftObjectPtr<UObject>& node : Nodes) {
		UObject* Obj = node.Get();
		if (Obj && Obj->Implements<UFINNetworkComponent>() && IFINNetworkComponent::Execute_GetID(Obj) == ID && IFINNetworkComponent::Execute_AccessPermitted(Obj, ReqID)) {
			return Obj;
		}
	}

	return nullptr;
}

TSet<UObject*> UFINNetworkCircuit::FindComponentsByNick(const FString& Nick, const TScriptInterface<IFINNetworkComponent>& Requester) {
	FGuid ReqID = (Requester) ? IFINNetworkComponent::Execute_GetID(Requester.GetObject()) : FGuid();
	TSet<UObject*> Comps;
	for (const TSoftObjectPtr<UObject>& Node : Nodes) {
		UObject* Obj = Node.Get();
		if (Obj && Obj->Implements<UFINNetworkComponent>() && IFINNetworkComponent::Execute_HasNick(Obj, Nick) && IFINNetworkComponent::Execute_AccessPermitted(Obj, ReqID)) Comps.Add(Obj);
	}

	return Comps;
}

TSet<UObject*> UFINNetworkCircuit::GetComponents() {
	TSet<UObject*> Comps;
	for (const TSoftObjectPtr<UObject>& Node : Nodes) {
		UObject* Obj = Node.Get();
		if (Obj->Implements<UFINNetworkComponent>()) Comps.Add(Obj);
	}
	return Comps;
}

bool UFINNetworkCircuit::IsNodeConnected(const TScriptInterface<IFINNetworkCircuitNode>& Start, const TScriptInterface<IFINNetworkCircuitNode>& Node) {
	TSet<UObject*> Searched;
	return IsNodeConnected_Internal(Start, Node, Searched);
}

void UFINNetworkCircuit::DisconnectNodes(const TScriptInterface<IFINNetworkCircuitNode>& A, const TScriptInterface<IFINNetworkCircuitNode>& B) {
	if (!IsNodeConnected(A, B)) {
		UFINNetworkCircuit* CircuitA = NewObject<UFINNetworkCircuit>();
		IFINNetworkCircuitNode::Execute_SetCircuit(A.GetObject(), CircuitA);
		CircuitA->Recalculate(A);

		UFINNetworkCircuit* CircuitB = IFINNetworkCircuitNode::Execute_GetCircuit(B.GetObject());
		
		TSet<UObject*> NodesA;
		for (const TSoftObjectPtr<UObject>& Node : CircuitA->Nodes) {
			UObject* Obj = Node.Get();
			if (Obj) NodesA.Add(Obj);
		}
		TSet<UObject*> NodesB;
		for (const TSoftObjectPtr<UObject>& Node : CircuitB->Nodes) {
			UObject* Obj = Node.Get();
			if (Obj) NodesB.Add(Obj);
		}

		for (const TSoftObjectPtr<UObject>& Node : CircuitB->Nodes) {
			if (CircuitB->Nodes.Contains(Node)) continue;
			IFINNetworkCircuitNode::Execute_NotifyNetworkUpdate(Node.Get(), 1, NodesA);
		}
		CircuitB->Recalculate(B);
		for (const TSoftObjectPtr<UObject>& Node : CircuitA->Nodes) {
			if (CircuitA->Nodes.Contains(Node)) continue;
			IFINNetworkCircuitNode::Execute_NotifyNetworkUpdate(Node.Get(), 1, NodesB);
		}
	}
}

void UFINNetworkCircuit::ConnectNodes(const TScriptInterface<IFINNetworkCircuitNode>& A, const TScriptInterface<IFINNetworkCircuitNode>& B) {
	UFINNetworkCircuit* CircuitA = IFINNetworkCircuitNode::Execute_GetCircuit(A.GetObject());
	UFINNetworkCircuit* CircuitB = IFINNetworkCircuitNode::Execute_GetCircuit(B.GetObject());
	if (!CircuitB) {
		CircuitB = NewObject<UFINNetworkCircuit>();
		IFINNetworkCircuitNode::Execute_SetCircuit(B.GetObject(), CircuitB);
		CircuitB->Recalculate(B);
	}
	if (CircuitA) {
		if (CircuitA != CircuitB) {
			IFINNetworkCircuitNode::Execute_SetCircuit(A.GetObject(), CircuitA = *CircuitA + CircuitB);
			IFINNetworkCircuitNode::Execute_SetCircuit(B.GetObject(), CircuitA);
		}
	} else {
		IFINNetworkCircuitNode::Execute_SetCircuit(A.GetObject(), CircuitB);
		CircuitB->Recalculate(A);
	}
}

bool UFINNetworkCircuit::IsNodeConnected_Internal(const TScriptInterface<IFINNetworkCircuitNode>& Self, const TScriptInterface<IFINNetworkCircuitNode>& Node, TSet<UObject*>& Searched) {
	if (Searched.Contains(Self.GetObject())) return false;
	Searched.Add(Self.GetObject());

	if (Self == Node) return true;
	
	for (TScriptInterface<IFINNetworkCircuitNode> Connected : IFINNetworkCircuitNode::Execute_GetConnected(Self.GetObject())) {
		if (IsNodeConnected_Internal(Connected, Node, Searched)) return true;
	}

	return false;
}
