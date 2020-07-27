#include "FINNetworkConnectionComponent.h"

#include "FINNetworkCable.h"
#include "FINNetworkCircuit.h"

TSet<UObject*> UFINNetworkConnectionComponent::GetConnected_Implementation() const {
	TSet<UObject*> Connected;
	for (const TSoftObjectPtr<UObject>& Node : ConnectedNodes) {
		UObject* Obj = Node.Get();
		if (Obj) Connected.Add(Obj);
	}
	for (AFINNetworkCable* Cable : ConnectedCables) {
		Connected.Add(Cable->Connector1);
		Connected.Add(Cable->Connector2);
	}
	return Connected;
}

UFINNetworkCircuit* UFINNetworkConnectionComponent::GetCircuit_Implementation() const {
	return Circuit;
}

void UFINNetworkConnectionComponent::SetCircuit_Implementation(UFINNetworkCircuit* Circuit) {
	this->Circuit = Circuit;
}

void UFINNetworkConnectionComponent::NotifyNetworkUpdate_Implementation(int Type, const TSet<UObject*>& Nodes) {}

void UFINNetworkConnectionComponent::AddConnectedNode(TScriptInterface<IFINNetworkCircuitNode> Node) {
	if (ConnectedNodes.Contains(Node.GetObject())) return;

	ConnectedNodes.Add(Node.GetObject());
	UFINNetworkConnectionComponent* Obj = Cast<UFINNetworkConnectionComponent>(Node.GetObject());
	if (Obj) Obj->AddConnectedNode(this);

	UFINNetworkCircuit::ConnectNodes(this, Node);
}

void UFINNetworkConnectionComponent::RemoveConnectedNode(TScriptInterface<IFINNetworkCircuitNode> Node) {
	if (!ConnectedNodes.Contains(Node.GetObject())) return;

	ConnectedNodes.Remove(Node.GetObject());
	UFINNetworkConnectionComponent* Obj = Cast<UFINNetworkConnectionComponent>(Node.GetObject());
	if (Obj) Obj->ConnectedNodes.Remove(this);
	
	UFINNetworkCircuit::DisconnectNodes(this, Node);
}

bool UFINNetworkConnectionComponent::AddConnectedCable(AFINNetworkCable* Cable) {
	if (MaxCables >= 0 && MaxCables <= ConnectedCables.Num()) return false;
	if (ConnectedCables.Contains(Cable)) return true;
	
	ConnectedCables.Add(Cable);

	UFINNetworkConnectionComponent* OtherConnector = (Cable->Connector1 == this) ? ((Cable->Connector2 == this) ? nullptr : Cable->Connector2) : Cable->Connector1;
	OtherConnector->AddConnectedCable(Cable);

	UFINNetworkCircuit::ConnectNodes(this, OtherConnector);

	return true;
}

void UFINNetworkConnectionComponent::RemoveConnectedCable(AFINNetworkCable* Cable) {
	if (!ConnectedCables.Contains(Cable)) return;

	if (ConnectedCables.Remove(Cable) > 0) {
		UFINNetworkConnectionComponent* OtherConnector = (Cable->Connector1 == this) ? Cable->Connector2 : Cable->Connector1;
		OtherConnector->ConnectedCables.Remove(Cable);

		UFINNetworkCircuit::DisconnectNodes(this, OtherConnector);
	}
}

bool UFINNetworkConnectionComponent::IsConnected(const TScriptInterface<IFINNetworkCircuitNode>& Node) const {
	if (ConnectedNodes.Contains(Node.GetObject())) return true;
	for (AFINNetworkCable* Cable : ConnectedCables) {
		if (Cable->Connector1 == Node.GetObject() || Cable->Connector2 == Node.GetObject()) return true;
	}
	return false;
}