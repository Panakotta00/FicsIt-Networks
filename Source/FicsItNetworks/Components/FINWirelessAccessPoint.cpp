#include "FINWirelessAccessPoint.h"

#include "FicsItNetworks/FicsItNetworksModule.h"
#include "FicsItNetworks/Network/FINNetworkCircuit.h"


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

	AFINWirelessSubsystem::Get(GetWorld())->RecalculateWirelessConnections();
}

void AFINWirelessAccessPoint::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
	HandledMessages.Empty();
}

void AFINWirelessAccessPoint::EndPlay(const EEndPlayReason::Type EndPlayReason) {
	Super::EndPlay(EndPlayReason);

	if (EndPlayReason == EEndPlayReason::Destroyed) {
		AFINWirelessSubsystem::Get(GetWorld())->RecalculateWirelessConnections();
	}
}

bool AFINWirelessAccessPoint::ShouldSave_Implementation() const {
	return true;
}

void AFINWirelessAccessPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINWirelessAccessPoint, AttachedTower);
}

TArray<UFINWirelessAccessPointConnection*> AFINWirelessAccessPoint::GetAvailableWirelessConnections() {
	TArray<UFINWirelessAccessPointConnection*> FoundAccessPoints;
	
	if (!IsValid(AttachedTower)) return FoundAccessPoints;

	const float AttachedRange = GetWirelessRange(AttachedTower);
	
	TArray<AActor*> RadarTowers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGBuildableRadarTower::StaticClass(), RadarTowers);
	// The script could be called after an "EndPlay" event. In this case the object is still returned even if being destroyed
	RadarTowers = RadarTowers.FilterByPredicate([](const AActor* Tower) {
		return !Tower->IsActorBeingDestroyed();
	});
	UE_LOG(LogFicsItNetworks, Log, TEXT("Found %d Radar Towers"), RadarTowers.Num());

	TArray<AActor*> AccessPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFINWirelessAccessPoint::StaticClass(), AccessPoints);
	AccessPoints = AccessPoints.FilterByPredicate([](const AActor* AccessPoint) {
		return !AccessPoint->IsActorBeingDestroyed();
	});
	UE_LOG(LogFicsItNetworks, Log, TEXT("Found %d Access Points"), AccessPoints.Num());
	
	// We register the accesspoint-radartower connection
	TMap<AFGBuildableRadarTower*, AFINWirelessAccessPoint*> AccessPointsAttachments;
	for (AActor* Actor : AccessPoints) {
		if (const auto AccessPoint = Cast<AFINWirelessAccessPoint>(Actor); IsValid(AccessPoint->AttachedTower)) {
			AccessPointsAttachments.Add(AccessPoint->AttachedTower, AccessPoint);
		}
	}

	// Sort all the towers putting the nearest first. This way we can check all the previous towers
	// For a connection to the next tower.
	Algo::SortBy(RadarTowers, [this](const AActor* Actor) {
		return FVector::DistSquared(Actor->GetActorLocation(), AttachedTower->GetActorLocation());
	});

	for (AActor* Actor : RadarTowers) {
		const auto TargetTower = Cast<AFGBuildableRadarTower>(Actor);
		const auto TargetRange = GetWirelessRange(TargetTower);
		
		const auto Distance = TargetTower->GetDistanceTo(AttachedTower);
		const auto AccessPoint = AccessPointsAttachments.Find(TargetTower);
		const bool IsConnected = AccessPoint != nullptr;

		// If RadarTower is not inside the AccessPoint range, it could still be connected through
		// repeaters antennas (WAP1 -> WAP2 -> WAP3).
		// We don't need this information for game logic, since repetition is handled by the network circuit itself,
		// however we store the information to display it in the UI
		bool IsRepeated = false;
		bool IsInRange = (TargetRange + AttachedRange) > Distance;
		if (!IsInRange) {
			for (const auto Repeater : FoundAccessPoints) {
				if (!Repeater->IsConnected || !Repeater->IsInRange) continue;
				
				const auto RepeaterDistance = TargetTower->GetDistanceTo(Repeater->AccessPoint->AttachedTower);
				
				if (TargetRange + GetWirelessRange(Repeater->AccessPoint->AttachedTower) > RepeaterDistance) {
					IsInRange = true;
					IsRepeated = true;
					break;
				}
			}
		}

		const auto FoundAccessPoint = NewObject<UFINWirelessAccessPointConnection>();
		FoundAccessPoint->RadarTower = TargetTower;
		FoundAccessPoint->AccessPoint = IsConnected ?  *AccessPoint : nullptr;
		FoundAccessPoint->Distance = Distance;
		FoundAccessPoint->IsInRange = IsInRange;
		FoundAccessPoint->IsConnected = IsConnected;
		FoundAccessPoint->IsRepeated = IsRepeated;
		
		FoundAccessPoint->ActorRepresentation = NewObject<UFINWirelessAccessPointActorRepresentation>();
		FoundAccessPoint->ActorRepresentation->Setup(FoundAccessPoint);
		
		FoundAccessPoints.Add(FoundAccessPoint);
	}
	
	return FoundAccessPoints;
}

/**
 * The wireless direction specifies if this message is "internal" (from the current circuit) or "external" (from wireless
 * network). If it's coming from the inside, we don't need to propagate it to the components of the circuit.
 */
bool AFINWirelessAccessPoint::HandleMessage(EFINWirelessDirection Direction, const FGuid& ID, const FGuid& Sender, const FGuid& Receiver, int Port, const TArray<FFINAnyNetworkValue>& Data) {
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
	for (const auto Connection : mConnectedAccessPoints) {
		if (IsValid(Connection->AccessPoint.Get())) {
			Connection->AccessPoint->HandleMessage(EFINWirelessDirection::FromWireless, ID, Sender, Receiver, Port, Data);
			bSent = true;
		}
	}
	
	return bSent;
}


