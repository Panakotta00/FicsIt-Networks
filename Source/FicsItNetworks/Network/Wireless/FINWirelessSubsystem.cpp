// Copyright Coffee Stain Studios. All Rights Reserved.


#include "FINWirelessSubsystem.h"

#include "FicsItNetworks/FicsItNetworksModule.h"
#include "FicsItNetworks/Components/FINWirelessAccessPoint.h"
#include "Subsystem/SubsystemActorManager.h"


AFINWirelessSubsystem::AFINWirelessSubsystem() {
	SetReplicates(true);
	bAlwaysRelevant = true;
}

void AFINWirelessSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFINWirelessSubsystem, bDummy);
}

/**
 * Gets the cached access points and make sure they are valid before using them.
 */
TArray<AFINWirelessAccessPoint*> AFINWirelessSubsystem::GetAccessPoints() {
	TArray<AFINWirelessAccessPoint*> AccessPoints;

	for (const auto Actor : CachedAccessPoints) {
		if (const auto AccessPoint = Cast<AFINWirelessAccessPoint>(Actor); IsValid(AccessPoint)) {
			AccessPoints.Emplace(AccessPoint);
		}
	}

	return AccessPoints;
}

/**
 * Update each access point, storing the reachable access points from the current one.
 * We use this mechanism to avoid recomputing the network wireless connections each tick.
 *
 * TODO Right now the algorithm is simple and reuses the GetAvailableWirelessConnections method from the AccessPoint.
 * We could refactor it with some optimization like computing the topology of the network and only update the connections,
 * but it's not a big deal given the small amount of access points needed in the map.
 */
void AFINWirelessSubsystem::RecalculateWirelessConnections() {
	// Only the host keeps updated the wireless connections. Guest clients use the replicated data
	// inside the single Access Points.
	if (!HasAuthority()) return;
	
	UE_LOG(LogFicsItNetworks, Log, TEXT("Recalculating Wireless Connections"));

	// Update cache
	CacheTowersAndAccessPoints();
	
	const auto AccessPoints = GetAccessPoints();
	for (const auto AccessPoint : AccessPoints) {
		AccessPoint->mWirelessConnections =  {};

		const auto Connections = GetAvailableConnections(AccessPoint);
		for (const auto Connection : Connections) {
			if (Connection->CanCommunicate()) {
				AccessPoint->mWirelessConnections.Emplace(Connection);
			}
		}
	}
}

/**
 * Finds all the connections between the selected access point and the others.
 * All data (towers & waps) are cached in the subsystem.
 */
TArray<UFINWirelessAccessPointConnection*> AFINWirelessSubsystem::GetAvailableConnections(AFINWirelessAccessPoint* CurrentAccessPoint) {
	TArray<UFINWirelessAccessPointConnection*> FoundConnections;

	const auto AttachedTower = CurrentAccessPoint->AttachedTower;
	if (!IsValid(AttachedTower)) return FoundConnections;

	const float AttachedRange = AFINWirelessAccessPoint::GetWirelessRange(AttachedTower);
	
	// We register the accesspoint-radartower connection
	TMap<AFGBuildableRadarTower*, AFINWirelessAccessPoint*> AccessPointsAttachments;
	for (AActor* Actor : CachedAccessPoints) {
		if (const auto AccessPoint = Cast<AFINWirelessAccessPoint>(Actor); IsValid(AccessPoint) && IsValid(AccessPoint->AttachedTower)) {
			AccessPointsAttachments.Add(AccessPoint->AttachedTower, AccessPoint);
		}
	}

	// Sort all the towers putting the nearest first. This way we can check all the previous towers
	// For a connection to the next tower.
	TArray<AActor*> RadarTowers = TArray<AActor*>(CachedRadarTowers);
	RadarTowers = RadarTowers.FilterByPredicate([](const AActor* Actor) {
		return IsValid(Actor);
	});
	UE_LOG(LogFicsItNetworks, Log, TEXT("Found %d (%d valid) radar towers"), CachedRadarTowers.Num(), RadarTowers.Num());
	Algo::SortBy(RadarTowers, [AttachedTower](const AActor* Actor) {
		return FVector::DistSquared(Actor->GetActorLocation(), AttachedTower->GetActorLocation());
	});

	for (AActor* Actor : RadarTowers) {
		const auto TargetTower = Cast<AFGBuildableRadarTower>(Actor);
		const auto TargetRange = AFINWirelessAccessPoint::GetWirelessRange(TargetTower);
		
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
			for (const auto Repeater : FoundConnections) {
				if (!Repeater->Data.IsConnected || !Repeater->Data.IsInRange) continue;
				
				const auto RepeaterDistance = TargetTower->GetDistanceTo(Repeater->AccessPoint->AttachedTower);
				
				if (TargetRange + AFINWirelessAccessPoint::GetWirelessRange(Repeater->AccessPoint->AttachedTower) > RepeaterDistance) {
					IsInRange = true;
					IsRepeated = true;
					break;
				}
			}
		}

		const auto FoundConnection = NewObject<UFINWirelessAccessPointConnection>();
		FoundConnection->RadarTower = TargetTower;
		FoundConnection->AccessPoint = IsConnected ?  *AccessPoint : nullptr;
		FoundConnection->Data.Distance = Distance;
		FoundConnection->Data.IsInRange = IsInRange;
		FoundConnection->Data.IsConnected = IsConnected;
		FoundConnection->Data.IsRepeated = IsRepeated;
		FoundConnection->Data.IsSelf = IsConnected && CurrentAccessPoint == *AccessPoint;

		// Cached data for replication
		FoundConnection->Data.RepresentationLocation = TargetTower->GetActorLocation();
		FoundConnection->Data.RepresentationText = TargetTower->GetRepresentationText();
		FoundConnection->Data.RadarTowerHasPower = TargetTower->HasPower();
		
		FoundConnection->SetupActorRepresentation();
		
		FoundConnections.Add(FoundConnection);
	}
	
	return FoundConnections;
}

void AFINWirelessSubsystem::BeginPlay() {
	Super::BeginPlay();

	UE_LOG(LogFicsItNetworks, Log, TEXT("Wireless Subsystem started"));
}

void AFINWirelessSubsystem::CacheTowersAndAccessPoints() {
	UE_LOG(LogFicsItNetworks, Log, TEXT("Wireless Subsystem: caching towers & wap"));

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFGBuildableRadarTower::StaticClass(), CachedRadarTowers);
	// The script could be called after an "EndPlay" event. In this case the object is still returned even if being destroyed
	CachedRadarTowers = CachedRadarTowers.FilterByPredicate([](const AActor* Tower) {
		return !Tower->IsActorBeingDestroyed();
	});
	UE_LOG(LogFicsItNetworks, Log, TEXT("Found %d Radar Towers"), CachedRadarTowers.Num());
	
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFINWirelessAccessPoint::StaticClass(), CachedAccessPoints);
	CachedAccessPoints = CachedAccessPoints.FilterByPredicate([](const AActor* AccessPoint) {
		return !AccessPoint->IsActorBeingDestroyed();
	});
	UE_LOG(LogFicsItNetworks, Log, TEXT("Found %d Access Points"), CachedAccessPoints.Num());
}

AFINWirelessSubsystem* AFINWirelessSubsystem::Get(UObject* WorldContext) {
	const UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINWirelessSubsystem>();
}
