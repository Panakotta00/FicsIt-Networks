#include "FINNetworkCircuit.h"
#include "FINNetworkComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

void AFINNetworkCircuit::AddNodeRecursive(TArray<TScriptInterface<IFINNetworkCircuitNode>>& Added, TScriptInterface<IFINNetworkCircuitNode> Add) {
	if (Add.GetObject() && !Added.Contains(Add)) {
		Added.Add(Add);
		Nodes.AddUnique(Add.GetObject());
		IFINNetworkCircuitNode::Execute_SetCircuit(Add.GetObject(), this);
		TSet<UObject*> ConNodes = IFINNetworkCircuitNode::Execute_GetConnected(Add.GetObject());
		for (UObject* Node : ConNodes) {
			AddNodeRecursive(Added, Node);
		}
	}
}

AFINNetworkCircuit::AFINNetworkCircuit() {
	bReplicates = true;
	bAlwaysRelevant = true;
}

AFINNetworkCircuit::~AFINNetworkCircuit() {}

bool AFINNetworkCircuit::IsSupportedForNetworking() const {
	return true;
}

void AFINNetworkCircuit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINNetworkCircuit, Nodes);
}

AFINNetworkCircuit* AFINNetworkCircuit::operator+(AFINNetworkCircuit* Circuit) {
	if (this == Circuit || !IsValid(Circuit)) return this;

	AFINNetworkCircuit* From = Circuit;
	AFINNetworkCircuit* To = this;

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
		if (!Obj) continue;
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
		if (Obj) IFINNetworkCircuitNode::Execute_NotifyNetworkUpdate(Obj, 0, FromNodes);
	}

	for (const TSoftObjectPtr<UObject>& Node : From->Nodes) To->Nodes.AddUnique(Node.Get() );

	return To;
}

void AFINNetworkCircuit::Recalculate(const TScriptInterface<IFINNetworkCircuitNode>& Node) {
	Nodes.Empty();

	TArray<TScriptInterface<IFINNetworkCircuitNode>> Added;
	AddNodeRecursive(Added, Node);
}

bool AFINNetworkCircuit::HasNode(const TScriptInterface<IFINNetworkCircuitNode>& Node) {
	return Nodes.Find(Node.GetObject()) != INDEX_NONE;
}

TScriptInterface<IFINNetworkComponent> AFINNetworkCircuit::FindComponent(const FGuid& ID, const TScriptInterface<IFINNetworkComponent>& Requester) {
	FGuid ReqID = (Requester) ? IFINNetworkComponent::Execute_GetID(Requester.GetObject()) : FGuid();
	for (const TSoftObjectPtr<UObject>& node : Nodes) {
		UObject* Obj = node.Get();
		if (Obj && Obj->Implements<UFINNetworkComponent>() && IFINNetworkComponent::Execute_GetID(Obj) == ID && IFINNetworkComponent::Execute_AccessPermitted(Obj, ReqID)) {
			return Obj;
		}
	}

	return nullptr;
}

TSet<UObject*> AFINNetworkCircuit::FindComponentsByNick(const FString& Nick, const TScriptInterface<IFINNetworkComponent>& Requester) {
	FGuid ReqID = (Requester) ? IFINNetworkComponent::Execute_GetID(Requester.GetObject()) : FGuid();
	TSet<UObject*> Comps;
	for (const TSoftObjectPtr<UObject>& Node : Nodes) {
		UObject* Obj = Node.Get();
		if (Obj && Obj->Implements<UFINNetworkComponent>() && IFINNetworkComponent::Execute_HasNick(Obj, Nick) && IFINNetworkComponent::Execute_AccessPermitted(Obj, ReqID)) Comps.Add(Obj);
	}

	return Comps;
}

TSet<UObject*> AFINNetworkCircuit::GetComponents() {
	TSet<UObject*> Comps;
	for (const TSoftObjectPtr<UObject>& Node : Nodes) {
		UObject* Obj = Node.Get();
		if (Obj && Obj->Implements<UFINNetworkComponent>()) Comps.Add(Obj);
	}
	return Comps;
}

bool AFINNetworkCircuit::IsNodeConnected(const TScriptInterface<IFINNetworkCircuitNode>& Start, const TScriptInterface<IFINNetworkCircuitNode>& Node) {
	TSet<UObject*> Searched;
	return IsNodeConnected_Internal(Start, Node, Searched);
}

void AFINNetworkCircuit::DisconnectNodes(UObject* WorldContext, const TScriptInterface<IFINNetworkCircuitNode>& A, const TScriptInterface<IFINNetworkCircuitNode>& B) {
	if (!IsNodeConnected(A, B)) {
		AFINNetworkCircuit* CircuitA = IFINNetworkCircuitNode::Execute_GetCircuit(A.GetObject());
		AFINNetworkCircuit* CircuitB = IFINNetworkCircuitNode::Execute_GetCircuit(B.GetObject());
		if (CircuitA != CircuitB) return;
		
		CircuitA = WorldContext->GetWorld()->SpawnActor<AFINNetworkCircuit>();
		IFINNetworkCircuitNode::Execute_SetCircuit(A.GetObject(), CircuitA);
		CircuitA->Recalculate(A);

		
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

void AFINNetworkCircuit::ConnectNodes(UObject* WorldContext, const TScriptInterface<IFINNetworkCircuitNode>& A, const TScriptInterface<IFINNetworkCircuitNode>& B) {
	AFINNetworkCircuit* CircuitA = IFINNetworkCircuitNode::Execute_GetCircuit(A.GetObject());
	AFINNetworkCircuit* CircuitB = IFINNetworkCircuitNode::Execute_GetCircuit(B.GetObject());
	if (!CircuitB) {
		FActorSpawnParameters Params;
		Params.bNoFail = true;
		CircuitB = WorldContext->GetWorld()->SpawnActor<AFINNetworkCircuit>(Params);
		check(CircuitB);
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

bool AFINNetworkCircuit::IsNodeConnected_Internal(const TScriptInterface<IFINNetworkCircuitNode>& Self, const TScriptInterface<IFINNetworkCircuitNode>& Node, TSet<UObject*>& Searched) {
	if (Searched.Contains(Self.GetObject())) return false;
	Searched.Add(Self.GetObject());

	if (Self == Node) return true;
	
	for (TScriptInterface<IFINNetworkCircuitNode> Connected : IFINNetworkCircuitNode::Execute_GetConnected(Self.GetObject())) {
		if (IsNodeConnected_Internal(Connected, Node, Searched)) return true;
	}

	return false;
}
