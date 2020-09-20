#include "FINAdvancedNetworkConnectionComponent.h"

#include "FINNetworkCircuit.h"
#include "UnrealNetwork.h"

UFINAdvancedNetworkConnectionComponent::UFINAdvancedNetworkConnectionComponent() {
	SetIsReplicated(true);
}

UFINAdvancedNetworkConnectionComponent::~UFINAdvancedNetworkConnectionComponent() {}

void UFINAdvancedNetworkConnectionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFINAdvancedNetworkConnectionComponent, ID);
	DOREPLIFETIME(UFINAdvancedNetworkConnectionComponent, Nick);
}

void UFINAdvancedNetworkConnectionComponent::BeginPlay() {
	Super::BeginPlay();
	
	if (bOuterAsRedirect) RedirectionObject = GetOuter();

	if (GetOwner()->HasAuthority()) {
		if (!bIdCreated) {
			ID = FGuid::NewGuid();
			bIdCreated = true;
		}

		// setup circuit
		if (!Circuit) {
			Circuit = GetWorld()->SpawnActor<AFINNetworkCircuit>();
			Circuit->Recalculate(this);
		}
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

FGuid UFINAdvancedNetworkConnectionComponent::GetID_Implementation() const {
	return ID;
}

FString UFINAdvancedNetworkConnectionComponent::GetNick_Implementation() const {
	return Nick;
}

void UFINAdvancedNetworkConnectionComponent::SetNick_Implementation(const FString& NewNick) {
	Nick = NewNick;
	GetOwner()->ForceNetUpdate();
}

bool UFINAdvancedNetworkConnectionComponent::HasNick_Implementation(const FString& inNick) {
	return HasNickByNick(inNick, Execute_GetNick(this));
}

UObject* UFINAdvancedNetworkConnectionComponent::GetInstanceRedirect_Implementation() const {
	return RedirectionObject;
}

bool UFINAdvancedNetworkConnectionComponent::AccessPermitted_Implementation(FGuid inID) const {
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

void UFINAdvancedNetworkConnectionComponent::HandleSignal(const TFINDynamicStruct<FFINSignal>& Signal, const FFINNetworkTrace& Sender) {
	OnNetworkSignal.Broadcast(Signal, Sender);
}

bool UFINAdvancedNetworkConnectionComponent::IsPortOpen(int Port) {
	if (OnIsNetworkPortOpen.IsBound()) {
		return OnIsNetworkPortOpen.Execute(Port);
	}
	return false;
}

void UFINAdvancedNetworkConnectionComponent::HandleMessage(FGuid ID, FFINNetworkTrace Sender, FGuid Receiver, int Port, const ::TFINDynamicStruct<FFINParameterList>& Data) {
	OnNetworkMessageRecieved.Broadcast(ID, Sender, Receiver, Port, Data);
}

bool UFINAdvancedNetworkConnectionComponent::IsNetworkMessageRouter() const {
	if (OnIsNetworkRouter.IsBound()) {
		return OnIsNetworkRouter.Execute();
	}
	return false;
}

void UFINAdvancedNetworkConnectionComponent::netSig_NetworkUpdate_Implementation(int type, const FString& id) {}
