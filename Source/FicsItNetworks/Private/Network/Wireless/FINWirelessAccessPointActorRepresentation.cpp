#include "Network/Wireless/FINWirelessAccessPointActorRepresentation.h"
#include "Network/Wireless/FINWirelessAccessPointConnection.h"

UFINWirelessAccessPointActorRepresentation::UFINWirelessAccessPointActorRepresentation() {}

void UFINWirelessAccessPointActorRepresentation::Setup(UFINWirelessAccessPointConnection* Connection) {
	this->mIsLocal = true;
	this->mShouldShowInCompass = false;
	this->mShouldShowOnMap = true;
	this->mRepresentationText =  Connection->GetRepresentationText();
	this->mRepresentationColor = Connection->Data.IsConnected && Connection->Data.IsInRange ? Green : Connection->Data.IsInRange ? Orange : Red;
	this->mRepresentationTexture = LoadObject<UTexture2D>(NULL, TEXT("/FicsItNetworks/Components/WirelessAccessPoint/TXUI_FIN_Wifi_MapCompassIcon.TXUI_FIN_Wifi_MapCompassIcon"));
	//this->mRealActor = Connection->RadarTower.Get();
	this->mIsStatic = true;
	this->mActorRotation = FRotator::ZeroRotator;
	this->mActorLocation = Connection->GetRepresentationLocation();
	this->mRealActorLocation = Connection->GetRepresentationLocation();

	// We use RT_StartingPod since RT_Default destroys automatically the ActorRepresentation after 10 seconds
	// Cannot use RT_MapMarker since it opens the marker editor when clicked.
	this->mRepresentationType = ERepresentationType::RT_StartingPod;
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

