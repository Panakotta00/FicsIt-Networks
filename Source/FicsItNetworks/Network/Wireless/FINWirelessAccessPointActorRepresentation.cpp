// Copyright Coffee Stain Studios. All Rights Reserved.


#include "FINWirelessAccessPointActorRepresentation.h"
#include "FINWirelessAccessPointConnection.h"
#include "FicsItNetworks/FicsItNetworksModule.h"

UFINWirelessAccessPointActorRepresentation::UFINWirelessAccessPointActorRepresentation() {}

void UFINWirelessAccessPointActorRepresentation::Setup(const UFINWirelessAccessPointConnection* Connection) {
	this->mIsLocal = true;
	this->mShouldShowInCompass = false;
	this->mShouldShowOnMap = true;
	this->mRepresentationText =  Connection->RadarTower->GetRepresentationText();
	this->mRepresentationColor = Connection->IsConnected && Connection->IsInRange ? Green : Connection->IsInRange ? Orange : Red;
	this->mRepresentationTexture = LoadObject<UTexture2D>(NULL, TEXT("/FicsItNetworks/Components/WirelessAccessPoint/TXUI_FIN_Wifi_Icon.TXUI_FIN_Wifi_Icon"));
	//this->mRealActor = Connection->RadarTower.Get();
	this->mIsStatic = true;
	this->mActorRotation = FRotator::ZeroRotator;
	this->mActorLocation = Connection->RadarTower->GetActorLocation();
	this->mRealActorLocation = Connection->RadarTower->GetActorLocation();

	// We use MapMarker since RT_Default destroys automatically the ActorRepresentation after 10 seconds
	this->mRepresentationType = ERepresentationType::RT_MapMarker;
}

void UFINWirelessAccessPointActorRepresentation::SetupActorRepresentation(AActor* realActor, bool isLocal, float lifeSpan) {}

FVector UFINWirelessAccessPointActorRepresentation::GetActorLocation() const {
	return this->mRealActorLocation;
}

void UFINWirelessAccessPointActorRepresentation::TrySetupDestroyTimer(float lifeSpan) {
}

bool UFINWirelessAccessPointActorRepresentation::CanBeHighlighted() const {
	return true;
}

