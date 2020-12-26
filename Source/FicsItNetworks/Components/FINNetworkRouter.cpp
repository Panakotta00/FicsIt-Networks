#include "FINNetworkRouter.h"
#include "Network/FINAdvancedNetworkConnectionComponent.h"
#include "Network/FINNetworkCircuit.h"

AFINNetworkRouter::AFINNetworkRouter() {
	NetworkConnector1 = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector1");
	NetworkConnector1->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	
	NetworkConnector2 = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector2");
	NetworkConnector2->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

AFINNetworkRouter::~AFINNetworkRouter() {}
void AFINNetworkRouter::BeginPlay() {
	Super::BeginPlay();

	NetworkConnector1->OnIsNetworkRouter.BindLambda([]() {
        return true;
    });
	NetworkConnector1->OnIsNetworkPortOpen.BindLambda([this](int Port) {
        return PortList.Contains(Port) == bIsPortWhitelist;
    });
	NetworkConnector1->OnNetworkMessageRecieved.AddLambda([this](FGuid ID, FGuid Sender, FGuid Reciever, int Port, const TArray<FFINAnyNetworkValue>& Data) {
        NetMulti_OnMessageHandled(false, true);
        if (HandleMessage(IFINNetworkCircuitNode::Execute_GetCircuit(NetworkConnector2), ID, Sender, Reciever, Port, Data))
        	NetMulti_OnMessageHandled(true, false);
    });
	NetworkConnector2->OnIsNetworkRouter.BindLambda([]() {
        return true;
    });
	NetworkConnector2->OnIsNetworkPortOpen.BindLambda([this](int Port) {
        return PortList.Contains(Port) == bIsPortWhitelist;
    });
	NetworkConnector2->OnNetworkMessageRecieved.AddLambda([this](FGuid ID, FGuid Sender, FGuid Reciever, int Port, const TArray<FFINAnyNetworkValue>& Data) {
        NetMulti_OnMessageHandled(true, true);
        if (HandleMessage(IFINNetworkCircuitNode::Execute_GetCircuit(NetworkConnector1), ID, Sender, Reciever, Port, Data))
        	NetMulti_OnMessageHandled(false, false);
    });
}

void AFINNetworkRouter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	HandledMessages.Empty();
}

bool AFINNetworkRouter::HandleMessage(AFINNetworkCircuit* SendingCircuit, FGuid ID, FGuid Sender, FGuid Receiver, int Port, const TArray<FFINAnyNetworkValue>& Data) {
	if (HandledMessages.Contains(ID) || !SendingCircuit) return false;
	HandledMessages.Add(ID);
	if (AddrList.Contains(Sender.ToString()) != bIsAddrWhitelist) return false;
	bool bSent = false;
	if (Receiver.IsValid()) {
		UObject* Obj = SendingCircuit->FindComponent(Receiver, nullptr).GetObject();
		IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Obj);
		if (NetMsgI && NetMsgI->IsPortOpen(Port)) {
			NetMsgI->HandleMessage(ID, Sender, Receiver, Port, Data);
			bSent = true;
		} else if (!NetMsgI) {
			for (UObject* Router : SendingCircuit->GetComponents()) {
				IFINNetworkMessageInterface* MsgI = Cast<IFINNetworkMessageInterface>(Router);
				if (!MsgI || !MsgI->IsNetworkMessageRouter() || !MsgI->IsPortOpen(Port)) continue;
				MsgI->HandleMessage(ID, Sender, Receiver, Port, Data);
				bSent = true;
			}
		}
	} else {
		for (UObject* MsgHandler : SendingCircuit->GetComponents()) {
			IFINNetworkMessageInterface* MsgI = Cast<IFINNetworkMessageInterface>(MsgHandler);
			if (!MsgI || !MsgI->IsPortOpen(Port)) continue;
			MsgI->HandleMessage(ID, Sender, Receiver, Port, Data);
			bSent = true;
		}
	}
	return bSent;
}

void AFINNetworkRouter::NetMulti_OnMessageHandled_Implementation(bool bCon1or2, bool bSendOrReceive) {
	OnMessageHandled(bCon1or2, bSendOrReceive);
}

void AFINNetworkRouter::netFunc_setPortWhitelist(bool bInWhitelist) {
	bIsPortWhitelist = bInWhitelist;
}

bool AFINNetworkRouter::netFunc_getPortWhitelist() {
	return bIsPortWhitelist;
}

void AFINNetworkRouter::netFunc_addPortList(int port) {
	PortList.AddUnique(port);
}

void AFINNetworkRouter::netFunc_removePortList(int port) {
	PortList.Remove(port);
}

void AFINNetworkRouter::netFunc_setPortList(const TArray<int>& portList) {
	PortList = portList;
}

TArray<int> AFINNetworkRouter::netFunc_getPortList() {
	return PortList;
}

void AFINNetworkRouter::netFunc_setAddrWhitelist(bool bInWhitelist) {
	bIsAddrWhitelist = bInWhitelist;
}

bool AFINNetworkRouter::netFunc_getAddrWhitelist() {
	return bIsAddrWhitelist;
}

void AFINNetworkRouter::netFunc_addAddrList(const FString& addr) {
	AddrList.AddUnique(addr);
}

void AFINNetworkRouter::netFunc_removeAddrList(const FString& addr) {
	AddrList.Remove(addr);
}

void AFINNetworkRouter::netFunc_setAddrList(const TArray<FString>& list) {
	AddrList = list;
}

TArray<FString> AFINNetworkRouter::netFunc_getAddrList() {
	return AddrList;
}
