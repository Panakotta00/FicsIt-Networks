#include "FINWirelessAccessPoint.h"

#include "FGPlayerController.h"
#include "FGPowerInfoComponent.h"
#include "FicsItNetworks/FicsItNetworksModule.h"
#include "FicsItNetworks/Network/FINNetworkCircuit.h"
#include "FicsItNetworks/Network/Wireless/FINWirelessRCO.h"


AFINWirelessAccessPoint::AFINWirelessAccessPoint() {
	NetworkConnector1 = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector1");
	NetworkConnector1->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	NetworkConnector1->SetIsReplicated(true);
	NetworkConnector1->MaxCables = 1;
	
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

AFINWirelessAccessPoint::~AFINWirelessAccessPoint() {}

float AFINWirelessAccessPoint::GetWirelessRange(AFGBuildableRadarTower* TargetTower) {
	// Same formula as described here https://satisfactory.fandom.com/wiki/Radar_Tower
	return FMath::Min( TargetTower->GetTowerHeight() * 2.86f + 92000.0f, 212000.0f);
}

bool AFINWirelessAccessPoint::IsInWirelessRange(AFGBuildableRadarTower* Tower) {
	if (!IsValid(AttachedTower)) return false;
	
	const auto Distance = FVector::Dist(AttachedTower->GetActorLocation(), Tower->GetActorLocation());
	const auto SourceRange = GetWirelessRange(AttachedTower);
	const auto TargetRange = GetWirelessRange(Tower);

	return SourceRange + TargetRange > Distance;
}

// Binds connector events to this Access Point implementation
void AFINWirelessAccessPoint::BeginPlay() {
	Super::BeginPlay();

	NetworkConnector1->OnIsNetworkRouter.BindLambda([] {
		return true;
	});
	NetworkConnector1->OnIsNetworkPortOpen.BindLambda([](int Port) {
		return true;
	});
	// TODO: Add LED indicators
	NetworkConnector1->OnNetworkMessageRecieved.AddLambda([this](const FGuid& ID, const FGuid& Sender, const FGuid& Reciever, int Port, const TArray<FFINAnyNetworkValue>& Data) {
		// this->LampFlags |= FIN_NetRouter_Con1_Tx;
		if (HandleMessage(EFINWirelessDirection::FromCircuit, ID, Sender, Reciever, Port, Data)) {
			// this->LampFlags |= FIN_NetRouter_Con2_Rx;
		}
	});
	
	UE_LOG(LogFicsItNetworks, Log, TEXT("FINWirelessAccessPoint::BeginPlay"));
	if (HasAuthority()) {
		AFINWirelessSubsystem::Get(GetWorld())->RecalculateWirelessConnections();
	}
}

void AFINWirelessAccessPoint::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	HandledMessages.Empty();
}

void AFINWirelessAccessPoint::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed && HasAuthority()) {
		AFINWirelessSubsystem::Get(GetWorld())->RecalculateWirelessConnections();
	}
}

bool AFINWirelessAccessPoint::ShouldSave_Implementation() const {
	return true;
}

// Networking

void AFINWirelessAccessPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINWirelessAccessPoint, AttachedTower);
	DOREPLIFETIME(AFINWirelessAccessPoint, mAvailableWirelessConnectionsData);
}

/**
 * On Multiplayer Host, it just refreshes the UI data and then updates on data to be replicated.
 * On Multiplayer Guest, it requests (through RCO) to get the updated data from the host.
 */
void AFINWirelessAccessPoint::RefreshWirelessConnectionsData() {
	if (HasAuthority()) {
		const auto WirelessSubsystem = AFINWirelessSubsystem::Get(GetWorld());
		// TODO Refresh RadarTowers&WAPs? Should not be needed
		mDisplayedWirelessConnections = WirelessSubsystem->GetAvailableConnections(this);

		mAvailableWirelessConnectionsData.Reset();
		for (const auto Connection : mDisplayedWirelessConnections) {
			mAvailableWirelessConnectionsData.Add(Connection->Data);
		}

		// Replicates the connections data to guests
		ForceNetUpdate();
	}
	else {
		UFINWirelessRCO* RCO = Cast<UFINWirelessRCO>(Cast<AFGPlayerController>(GetWorld()->GetFirstPlayerController())->GetRemoteCallObjectOfClass(UFINWirelessRCO::StaticClass()));
		RCO->SubscribeWirelessAccessPointConnections(this, true);
	}
}

TArray<UFINWirelessAccessPointConnection*> AFINWirelessAccessPoint::GetDisplayedWirelessConnections() {
	RefreshWirelessConnectionsData();
	return mDisplayedWirelessConnections;
}

/**
 * A Radar Tower needs power in order to receive messages.
 * We check it here in order to avoid recalculating the connections everytime a tower gets powerered/unpowered.
 */
bool AFINWirelessAccessPoint::CanHandleMessages() {
	return IsValid(AttachedTower) && AttachedTower->HasPower();
}

/**
 * The wireless direction specifies if this message is "internal" (from the current circuit) or "external" (from wireless
 * network). If it's coming from the inside, we don't need to propagate it to the components of the circuit.
 */
bool AFINWirelessAccessPoint::HandleMessage(EFINWirelessDirection Direction, const FGuid& ID, const FGuid& Sender, const FGuid& Receiver, int Port, const TArray<FFINAnyNetworkValue>& Data) {
	if (!CanHandleMessages()) return false;

	const auto SendingCircuit = IFINNetworkCircuitNode::Execute_GetCircuit(NetworkConnector1);

	{
		FScopeLock Lock(&HandleMessageMutex);
		if (HandledMessages.Contains(ID) || !SendingCircuit) return false;
		HandledMessages.Add(ID);
	}

	bool bSent = false;

	if (Direction == EFINWirelessDirection::FromWireless) {
		if (Receiver.IsValid()) {
			UObject* Obj = SendingCircuit->FindComponent(Receiver, nullptr).GetObject();
			IFINNetworkMessageInterface* NetMsgI = Cast<IFINNetworkMessageInterface>(Obj);
			if (NetMsgI) {
				// send to specified component
				NetMsgI->HandleMessage(ID, Sender, Receiver, Port, Data);
				return true;
			}
		} else {
			// distribute to components
			for (UObject* MsgHandler : SendingCircuit->GetComponents()) {
				IFINNetworkMessageInterface* MsgI = Cast<IFINNetworkMessageInterface>(MsgHandler);
				if (!MsgI) continue;
				MsgI->HandleMessage(ID, Sender, Receiver, Port, Data);
				bSent = true;
			}
		}
	}

	// distribute over wireless routers
	for (const auto Connection : mWirelessConnections) {
		if (IsValid(Connection->AccessPoint.Get())) {
			Connection->AccessPoint->HandleMessage(EFINWirelessDirection::FromWireless, ID, Sender, Receiver, Port, Data);
			bSent = true;
		}
	}
	
	return bSent;
}


