#include "FINComputerNetworkCard.h"


#include "Network/FINVariadicParameterList.h"
#include "Network/Signals/FINSignalListener.h"
#include "Network/Signals/FINSmartSignal.h"

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
	arr.Add(ConnectedComponent);
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

bool AFINComputerNetworkCard::IsPortOpen(int Port) {
	return OpenPorts.Contains(Port);
}

void AFINComputerNetworkCard::HandleMessage(FFINNetworkTrace Sender, int Port, const TFINDynamicStruct<FFINParameterList>& Data) {
	for (const FFINNetworkTrace& Listener : Listeners) {
		Cast<IFINSignalListener>(*Listener)->HandleSignal(MakeShared<FFINNetworkMessageSignal>(Port, Data), Sender);
	}
}

void AFINComputerNetworkCard::netFunc_open(int port) {
	if (port < 0 || port > 1000) return;
	if (!OpenPorts.Contains(port)) OpenPorts.Add(port);
}

void AFINComputerNetworkCard::netFunc_close(int port) {
	OpenPorts.Remove(port);
}

void AFINComputerNetworkCard::netFunc_closeAll() {
	OpenPorts.Empty();
}

void AFINComputerNetworkCard::netFunc_send(FFINNetworkTrace Reciever, int port, FFINDynamicStructHolder args) {
	if (port < 0 || port > 1000) return;

	UObject* Obj = *Reciever;
	IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Obj);
	if (NetMsgI) {
		if (NetMsgI->IsPortOpen(port)) NetMsgI->HandleMessage(Reciever.Reverse(), port, args);
	}
}

void AFINComputerNetworkCard::netFunc_broadcast(int port, FFINDynamicStructHolder args) {
	if (port < 0 || port > 1000) return;
	for (UObject* Component : GetCircuit_Implementation()->GetComponents()) {
		IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Component);
		if (NetMsgI && NetMsgI->IsPortOpen(port)) {
			NetMsgI->HandleMessage(FFINNetworkTrace(this) / Component, port, args);
		}
	}
}

FFINNetworkMessageSignal::FFINNetworkMessageSignal(int Port, const TFINDynamicStruct<FFINParameterList>& Data) : FFINSignal("NetworkMessage"), Port(Port), Data(Data) {}

bool FFINNetworkMessageSignal::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);

	Ar << Port;
	Ar << Data;

	return true;
}

int FFINNetworkMessageSignal::operator>>(FFINValueReader& reader) const {
	reader << (FINInt)Port;
	return 1 + (**Data >> reader);
}
