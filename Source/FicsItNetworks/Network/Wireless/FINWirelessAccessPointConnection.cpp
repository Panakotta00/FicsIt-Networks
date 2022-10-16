#include "FINWirelessAccessPointConnection.h"

#include "FINWirelessAccessPointConnectionData.h"
#include "FicsItNetworks/Components/FINWirelessAccessPoint.h"

// void UFINWirelessAccessPointConnection::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
// 	DOREPLIFETIME(UFINWirelessAccessPointConnection, AccessPoint);
// 	DOREPLIFETIME(UFINWirelessAccessPointConnection, RadarTower);
// 	DOREPLIFETIME(UFINWirelessAccessPointConnection, Distance);
// 	DOREPLIFETIME(UFINWirelessAccessPointConnection, ActorRepresentation);
// 	DOREPLIFETIME(UFINWirelessAccessPointConnection, IsConnected);
// 	DOREPLIFETIME(UFINWirelessAccessPointConnection, IsRepeated);
// 	DOREPLIFETIME(UFINWirelessAccessPointConnection, IsInRange);
// }

void UFINWirelessAccessPointConnection::FromData(const FFINWirelessAccessPointConnectionData& NewData) {
	Data = NewData;
	SetupActorRepresentation();
}

void UFINWirelessAccessPointConnection::SetupActorRepresentation() {
	ActorRepresentation = NewObject<UFINWirelessAccessPointActorRepresentation>();
	ActorRepresentation->Setup(this);
}
