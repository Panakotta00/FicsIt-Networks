#include "Network/Wireless/FINWirelessAccessPointConnection.h"

void UFINWirelessAccessPointConnection::FromData(const FFINWirelessAccessPointConnectionData& NewData) {
	Data = NewData;
	SetupActorRepresentation();
}

void UFINWirelessAccessPointConnection::SetupActorRepresentation() {
	ActorRepresentation = NewObject<UFINWirelessAccessPointActorRepresentation>();
	ActorRepresentation->Setup(this);
}
