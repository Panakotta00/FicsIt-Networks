#include "FINComputerNetworkCard.h"

void AFINComputerNetworkCard::BeginPlay() {
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

FGuid AFINComputerNetworkCard::GetID_Implementation() const {
	return ID;
}

FString AFINComputerNetworkCard::GetNick_Implementation() const {
	return Nick;
}

void AFINComputerNetworkCard::SetNick_Implementation(const FString& nick) {
	Nick = nick;
}

bool AFINComputerNetworkCard::HasNick_Implementation(const FString& nick) {
	return HasNickByNick(nick, Execute_GetNick(this));
}

TSet<UObject*> AFINComputerNetworkCard::GetMerged_Implementation() const {
	return TSet<UObject*>();
}

TSet<UObject*> AFINComputerNetworkCard::GetConnected_Implementation() const {
	TSet<UObject*> arr;
	arr.Add(Computer);
	return arr;
}

FFINNetworkTrace AFINComputerNetworkCard::FindComponent_Implementation(FGuid guid) const {
	UFINNetworkCircuit* circuit = Execute_GetCircuit(this);
	return FFINNetworkTrace(const_cast<AFINComputerNetworkCard*>(this)) / circuit->FindComponent(guid);
}

UFINNetworkCircuit* AFINComputerNetworkCard::GetCircuit_Implementation() const {
	return Circuit;
}

void AFINComputerNetworkCard::SetCircuit_Implementation(UFINNetworkCircuit * circuit) {
	Circuit = circuit;
}

void AFINComputerNetworkCard::NotifyNetworkUpdate_Implementation(int type, const TSet<UObject*>& nodes) {
	
}
