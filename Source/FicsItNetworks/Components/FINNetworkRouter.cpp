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
        this->LampFlags |= FIN_NetRouter_Con1_Tx;
        if (HandleMessage(IFINNetworkCircuitNode::Execute_GetCircuit(NetworkConnector2), ID, Sender, Reciever, Port, Data))
        	this->LampFlags |= FIN_NetRouter_Con2_Rx;
    });
	NetworkConnector2->OnIsNetworkRouter.BindLambda([]() {
        return true;
    });
	NetworkConnector2->OnIsNetworkPortOpen.BindLambda([this](int Port) {
        return PortList.Contains(Port) == bIsPortWhitelist;
    });
	NetworkConnector2->OnNetworkMessageRecieved.AddLambda([this](FGuid ID, FGuid Sender, FGuid Reciever, int Port, const TArray<FFINAnyNetworkValue>& Data) {
        this->LampFlags |= FIN_NetRouter_Con2_Tx;
        if (HandleMessage(IFINNetworkCircuitNode::Execute_GetCircuit(NetworkConnector1), ID, Sender, Reciever, Port, Data))
	        this->LampFlags |= FIN_NetRouter_Con1_Rx;
    });
}

void AFINNetworkRouter::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	HandledMessages.Empty();

	if (LampFlags != FIN_NetRouter_None) {
		NetMulti_OnMessageHandled(LampFlags);
		LampFlags = FIN_NetRouter_None;
	}
}

bool AFINNetworkRouter::HandleMessage(AFINNetworkCircuit* SendingCircuit, FGuid ID, FGuid Sender, FGuid Receiver, int Port, const TArray<FFINAnyNetworkValue>& Data) {
	{
		FScopeLock Lock(&HandleMessageMutex);
		if (HandledMessages.Contains(ID) || !SendingCircuit) return false;
		HandledMessages.Add(ID);
	}
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

void AFINNetworkRouter::NetMulti_OnMessageHandled_Implementation(EFINNetworkRouterLampFlags Flags) {
	if (Flags & FIN_NetRouter_Con1_Rx) {
		OnMessageHandled(false, false);
	}
	if (Flags & FIN_NetRouter_Con1_Tx) {
		OnMessageHandled(false, true);
	}
	if (Flags & FIN_NetRouter_Con2_Rx) {
		OnMessageHandled(true, false);
	}
	if (Flags & FIN_NetRouter_Con2_Tx) {
		OnMessageHandled(true, true);
	}
}

void AFINNetworkRouter::netPropSet_isWhitelist(bool bInWhitelist) {
	bIsPortWhitelist = bInWhitelist;
}

bool AFINNetworkRouter::netPropGet_isWhitelist() {
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

void AFINNetworkRouter::netPropSet_isAddrWhitelist(bool bInWhitelist) {
	bIsAddrWhitelist = bInWhitelist;
}

bool AFINNetworkRouter::netPropGet_isAddrWhitelist() {
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
