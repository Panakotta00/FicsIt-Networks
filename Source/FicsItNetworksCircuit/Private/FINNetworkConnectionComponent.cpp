#include "FINNetworkConnectionComponent.h"

#include "Net/UnrealNetwork.h"
#include "FINNetworkCable.h"
#include "FINNetworkCircuit.h"
#include "Resources/FGBuildingDescriptor.h"

void UFINNetworkConnectionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFINNetworkConnectionComponent, Circuit);
}

void UFINNetworkConnectionComponent::InitializeComponent() {
	Super::InitializeComponent();
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
}

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

AFINNetworkCircuit* UFINNetworkConnectionComponent::GetCircuit_Implementation() const {
	return Circuit;
}

void UFINNetworkConnectionComponent::SetCircuit_Implementation(AFINNetworkCircuit* NewCircuit) {
	Circuit = NewCircuit;
	GetOwner()->ForceNetUpdate();
}

void UFINNetworkConnectionComponent::NotifyNetworkUpdate_Implementation(int Type, const TSet<UObject*>& Nodes) {}

void UFINNetworkConnectionComponent::AddConnectedNode(TScriptInterface<IFINNetworkCircuitNode> Node) {
#if WITH_EDITOR
	return;
#endif
	if (ConnectedNodes.Contains(Node.GetObject()) || !GetOwner()->HasAuthority()) return;

	ConnectedNodes.Add(Node.GetObject());
	UFINNetworkConnectionComponent* Obj = Cast<UFINNetworkConnectionComponent>(Node.GetObject());
	if (Obj) Obj->AddConnectedNode(this);

	AFINNetworkCircuit::ConnectNodes(this, this, Node);

	GetOwner()->ForceNetUpdate();
}

void UFINNetworkConnectionComponent::RemoveConnectedNode(TScriptInterface<IFINNetworkCircuitNode> Node) {
	if (!ConnectedNodes.Contains(Node.GetObject())) return;

	ConnectedNodes.Remove(Node.GetObject());
	UFINNetworkConnectionComponent* Obj = Cast<UFINNetworkConnectionComponent>(Node.GetObject());
	if (Obj) Obj->ConnectedNodes.Remove(this);
	
	AFINNetworkCircuit::DisconnectNodes(this, this, Node);

	GetOwner()->ForceNetUpdate();
}

TSet<AFINNetworkCable*> UFINNetworkConnectionComponent::GetConnectedCables() {
	return ConnectedCables;
}

bool UFINNetworkConnectionComponent::AddConnectedCable(AFINNetworkCable* Cable) {
	if (!IsValid(Cable) || (MaxCables >= 0 && MaxCables <= ConnectedCables.Num())) return false;
	if (ConnectedCables.Contains(Cable)) return true;
	
	ConnectedCables.Add(Cable);

	UFINNetworkConnectionComponent* OtherConnector = (Cable->Connector1 == this) ? ((Cable->Connector2 == this) ? nullptr : Cable->Connector2) : Cable->Connector1;
	if (OtherConnector) {
		OtherConnector->AddConnectedCable(Cable);
		AFINNetworkCircuit::ConnectNodes(this, this, OtherConnector);
	}

	GetOwner()->ForceNetUpdate();
	
	return true;
}

void UFINNetworkConnectionComponent::RemoveConnectedCable(AFINNetworkCable* Cable) {
	if (!ConnectedCables.Contains(Cable)) return;

	if (ConnectedCables.Remove(Cable) > 0) {
		UFINNetworkConnectionComponent* OtherConnector = (Cable->Connector1 == this) ? Cable->Connector2 : Cable->Connector1;
		if (OtherConnector) {
			OtherConnector->ConnectedCables.Remove(Cable);
			AFINNetworkCircuit::DisconnectNodes(OtherConnector->Circuit, this, OtherConnector);
		}
	}

	GetOwner()->ForceNetUpdate();
}

bool UFINNetworkConnectionComponent::IsConnected(const TScriptInterface<IFINNetworkCircuitNode>& Node) const {
	if (ConnectedNodes.Contains(Node.GetObject())) return true;
	for (AFINNetworkCable* Cable : ConnectedCables) {
		if (Cable->Connector1 == Node.GetObject() || Cable->Connector2 == Node.GetObject()) return true;
	}
	return false;
}

void UFINNetworkConnectionComponent::GetAllowedCableConnections(TArray<TSubclassOf<UFGBuildingDescriptor>>& OutAllowedConnections) const {
	OutAllowedConnections.Append(AllowedCableConnections);
	if (AllowedCableConnections.Num() < 1) OutAllowedConnections.Add(LoadClass<UFGBuildingDescriptor>(NULL, TEXT("/FicsItNetworks/Network/NetworkCable/BD_NetworkCable.BD_NetworkCable_C")));
}
