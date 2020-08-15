#include "FINComputerNetworkCard.h"

#include "Network/FINNetworkCircuit.h"
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
		Circuit->Recalculate(this);
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

UObject* AFINComputerNetworkCard::GetInstanceRedirect_Implementation() const {
	return nullptr;
}

bool AFINComputerNetworkCard::AccessPermitted_Implementation(FGuid ID) const {
	return ID == FGuid() || (ConnectedComponent && ID == IFINNetworkComponent::Execute_GetID(ConnectedComponent));
}

TSet<UObject*> AFINComputerNetworkCard::GetConnected_Implementation() const {
	TSet<UObject*> Arr;
	Arr.Add(ConnectedComponent);
	return Arr;
}

UFINNetworkCircuit* AFINComputerNetworkCard::GetCircuit_Implementation() const {
	return Circuit;
}

void AFINComputerNetworkCard::SetCircuit_Implementation(UFINNetworkCircuit * Circuit) {
	this->Circuit = Circuit;
}

void AFINComputerNetworkCard::NotifyNetworkUpdate_Implementation(int Type, const TSet<UObject*>& Nodes) {}

bool AFINComputerNetworkCard::IsPortOpen(int Port) {
	return OpenPorts.Contains(Port);
}

void AFINComputerNetworkCard::HandleMessage(FFINNetworkTrace Sender, int Port, const TFINDynamicStruct<FFINParameterList>& Data) {
	for (const FFINNetworkTrace& Listener : Listeners) {
		Cast<IFINSignalListener>(*Listener)->HandleSignal(FFINNetworkMessageSignal(IFINNetworkComponent::Execute_GetID(*Sender), Port, Data), Listener.Reverse());
	}
}

void AFINComputerNetworkCard::netFunc_open(int port) {
	if (port < 0 || port > 10000) return;
	if (!OpenPorts.Contains(port)) OpenPorts.Add(port);
}

void AFINComputerNetworkCard::netFunc_close(int port) {
	OpenPorts.Remove(port);
}

void AFINComputerNetworkCard::netFunc_closeAll() {
	OpenPorts.Empty();
}

void AFINComputerNetworkCard::netFunc_send(FString receiver, int port, FFINDynamicStructHolder args) {
	FFINNetworkCardArgChecker Reader;
	int argCount = args.Get<FFINParameterList>() >> Reader;
	if (Reader.Fail || port < 0 || port > 10000 || argCount > 7) return;

	FGuid receiverID;
	FGuid::Parse(receiver, receiverID);
	UObject* Obj = Circuit->FindComponent(receiverID, nullptr).GetObject();
	IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Obj);
	if (NetMsgI) {
		if (NetMsgI->IsPortOpen(port)) NetMsgI->HandleMessage(FFINNetworkTrace(Obj) / this, port, args);
	}
}

void AFINComputerNetworkCard::netFunc_broadcast(int port, FFINDynamicStructHolder args) {
	FFINNetworkCardArgChecker Reader;
	int argCount = args.Get<FFINParameterList>() >> Reader;
	if (Reader.Fail || port < 0 || port > 10000 || argCount > 7) return;
	for (UObject* Component : GetCircuit_Implementation()->GetComponents()) {
		IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Component);
		if (NetMsgI && NetMsgI->IsPortOpen(port)) {
			NetMsgI->HandleMessage(FFINNetworkTrace(Component) / this, port, args);
		}
	}
}

void FFINNetworkCardArgChecker::operator<<(const FINObj& Obj) {
	Fail = true;
}

void FFINNetworkCardArgChecker::operator<<(const FINTrace& Obj) {
	Fail = true;
}

void FFINNetworkCardArgChecker::operator<<(const FINStruct& Struct) {
	Fail = true;
}

FFINNetworkMessageSignal::FFINNetworkMessageSignal(FGuid Sender, int Port, const TFINDynamicStruct<FFINParameterList>& Data) : FFINSignal("NetworkMessage"), Sender(Sender), Port(Port), Data(Data) {}

bool FFINNetworkMessageSignal::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);

	Ar << Sender;
	Ar << Port;
	Ar << Data;

	return true;
}

int FFINNetworkMessageSignal::operator>>(FFINValueReader& reader) const {
	reader << Sender.ToString();
	reader << static_cast<FINInt>(Port);
	return 2 + (**Data >> reader);
}
