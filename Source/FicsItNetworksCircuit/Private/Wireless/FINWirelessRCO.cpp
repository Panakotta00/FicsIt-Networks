#include "Wireless/FINWirelessRCO.h"

#include "Buildables/FINWirelessAccessPoint.h"
#include "Net/UnrealNetwork.h"

void UFINWirelessRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFINWirelessRCO, bDummy);
}

void UFINWirelessRCO::SubscribeWirelessAccessPointConnections_Implementation(AFINWirelessAccessPoint* AccessPoint, bool bSubscribe) {
	// Since now we are on the Host, the function will recalculate the connections
	AccessPoint->RefreshWirelessConnectionsData();
}

bool UFINWirelessRCO::SubscribeWirelessAccessPointConnections_Validate(AFINWirelessAccessPoint* AccessPoint, bool bSubscribe) {
	return true;
}
