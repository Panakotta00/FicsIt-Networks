#include "FINWirelessAccessPointConnection.h"

#include "FINWirelessAccessPointConnectionData.h"
#include "FicsItNetworks/Components/FINWirelessAccessPoint.h"

void UFINWirelessAccessPointConnection::FromData(const FFINWirelessAccessPointConnectionData& NewData) {
	Data = NewData;
	SetupActorRepresentation();
}

void UFINWirelessAccessPointConnection::SetupActorRepresentation() {
	ActorRepresentation = NewObject<UFINWirelessAccessPointActorRepresentation>();
	ActorRepresentation->Setup(this);
}
