// Copyright Coffee Stain Studios. All Rights Reserved.


#include "FINWirelessSubsystem.h"

#include "FicsItNetworks/FicsItNetworksModule.h"
#include "FicsItNetworks/Components/FINWirelessAccessPoint.h"
#include "Subsystem/SubsystemActorManager.h"


AFINWirelessSubsystem::AFINWirelessSubsystem() {}

TArray<AFINWirelessAccessPoint*> AFINWirelessSubsystem::GetAccessPoints() {
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFINWirelessAccessPoint::StaticClass(), Actors);

	TArray<AFINWirelessAccessPoint*> AccessPoints;
	for (const auto Actor : Actors) {
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
	UE_LOG(LogFicsItNetworks, Log, TEXT("Recalculating Wireless Connections"));
	
	const auto AccessPoints = GetAccessPoints();

	for (const auto AccessPoint : AccessPoints) {
		AccessPoint->mConnectedAccessPoints =  {};

		const auto Connections = AccessPoint->GetAvailableWirelessConnections();
		for (const auto Connection : Connections) {
			if (
				Connection->IsConnected &&
				Connection->IsInRange &&
				!Connection->IsRepeated &&
				Connection->AccessPoint.Get() != AccessPoint
				) {
				AccessPoint->mConnectedAccessPoints.Emplace(Connection);
			}
		}
	}
}

void AFINWirelessSubsystem::BeginPlay() {
	Super::BeginPlay();

	UE_LOG(LogFicsItNetworks, Log, TEXT("Wireless Subsystem started"));
}

AFINWirelessSubsystem* AFINWirelessSubsystem::Get(UObject* WorldContext) {
	const UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINWirelessSubsystem>();
}
