#include "Wireless/FINWirelessAccessPointConnection.h"

#include "Buildables/FGBuildableRadarTower.h"
#include "Wireless/FINWirelessAccessPointActorRepresentation.h"

bool UFINWirelessAccessPointConnection::CanCommunicate() const {
	return Data.IsConnected && Data.IsInRange && !Data.IsRepeated && !Data.IsSelf;
}

FText UFINWirelessAccessPointConnection::GetRepresentationText() {
	return RadarTower == nullptr ? Data.RepresentationText : RadarTower->GetRepresentationText();
}

FVector UFINWirelessAccessPointConnection::GetRepresentationLocation() const {
	return RadarTower == nullptr ? Data.RepresentationLocation : RadarTower->GetActorLocation();
}

bool UFINWirelessAccessPointConnection::HasPower() const {
	return RadarTower == nullptr ? Data.RadarTowerHasPower : RadarTower->HasPower();
}

void UFINWirelessAccessPointConnection::FromData(const FFINWirelessAccessPointConnectionData& NewData) {
	Data = NewData;
	SetupActorRepresentation();
}

void UFINWirelessAccessPointConnection::SetupActorRepresentation() {
	ActorRepresentation = NewObject<UFINWirelessAccessPointActorRepresentation>();
	ActorRepresentation->Setup(this);
}
