#include "FINComputerNetworkCard.h"


#include "UnrealNetwork.h"
#include "Network/FINNetworkCircuit.h"
#include "Network/FINVariadicParameterList.h"
#include "Network/Signals/FINSignalListener.h"
#include "Network/Signals/FINSmartSignal.h"


void AFINComputerNetworkCard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerNetworkCard, ID);
	DOREPLIFETIME(AFINComputerNetworkCard, Nick);
}

AFINComputerNetworkCard::AFINComputerNetworkCard() {
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINComputerNetworkCard::BeginPlay() {
	Super::BeginPlay();
	
	if (!bIdCreated) {
		ID = FGuid::NewGuid();
		bIdCreated = true;
	}

	// setup circuit
	if (!Circuit && HasAuthority()) {
		Circuit = GetWorld()->SpawnActor<AFINNetworkCircuit>();
		Circuit->Recalculate(this);
	}
}

void AFINComputerNetworkCard::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	HandledMessages.Empty();
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

bool AFINComputerNetworkCard::AccessPermitted_Implementation(FGuid NewID) const {
	return NewID == FGuid() || (ConnectedComponent && NewID == IFINNetworkComponent::Execute_GetID(ConnectedComponent));
}

TSet<UObject*> AFINComputerNetworkCard::GetConnected_Implementation() const {
	TSet<UObject*> Arr;
	Arr.Add(ConnectedComponent);
	return Arr;
}

AFINNetworkCircuit* AFINComputerNetworkCard::GetCircuit_Implementation() const {
	return Circuit;
}

void AFINComputerNetworkCard::SetCircuit_Implementation(AFINNetworkCircuit* NewCircuit) {
	Circuit = NewCircuit;
}

void AFINComputerNetworkCard::NotifyNetworkUpdate_Implementation(int Type, const TSet<UObject*>& Nodes) {}

bool AFINComputerNetworkCard::IsPortOpen(int Port) {
	return OpenPorts.Contains(Port);
}

void AFINComputerNetworkCard::HandleMessage(FGuid ID, FGuid Sender, FGuid Receiver, int Port, const TFINDynamicStruct<FFINParameterList>& Data) {
	if (HandledMessages.Contains(ID)) return;
	HandledMessages.Add(ID);
	for (const FFINNetworkTrace& Listener : Listeners) {
		IFINSignalListener* L = Cast<IFINSignalListener>(*Listener);
		if (L) L->HandleSignal(FFINNetworkMessageSignal(Sender, Port, Data), Listener.Reverse());
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
	FGuid MsgID = FGuid::NewGuid();
	FGuid SenderID = Execute_GetID(this);
	if (NetMsgI) {
		if (NetMsgI->IsPortOpen(port)) NetMsgI->HandleMessage(MsgID, SenderID, receiverID, port, args);
	} else {
		for (UObject* Router : Circuit->GetComponents()) {
			IFINNetworkMessageInterface* MsgI = Cast<IFINNetworkMessageInterface>(Router);
			if (!MsgI || !MsgI->IsNetworkMessageRouter() || !MsgI->IsPortOpen(port)) continue;
			MsgI->HandleMessage(MsgID, SenderID, receiverID, port, args);
		}
	}
}

void AFINComputerNetworkCard::netFunc_broadcast(int port, FFINDynamicStructHolder args) {
	FFINNetworkCardArgChecker Reader;
	int argCount = args.Get<FFINParameterList>() >> Reader;
	if (Reader.Fail || port < 0 || port > 10000 || argCount > 7) return;
	FGuid MsgID = FGuid::NewGuid();
	FGuid SenderID = Execute_GetID(this);
	for (UObject* Component : GetCircuit_Implementation()->GetComponents()) {
		IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Component);
		if (NetMsgI && NetMsgI->IsPortOpen(port)) {
			NetMsgI->HandleMessage(MsgID, SenderID, FGuid(), port, args);
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
