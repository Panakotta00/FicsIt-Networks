#include "FINMCPNetworkCable.h"



AFINMCPNetworkCable::AFINMCPNetworkCable() {
}



void AFINMCPNetworkCable::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	UStaticMesh* cableMesh = LoadObject<UStaticMesh>(NULL, TEXT("/FicsItNetworks/Network/ThinNetworkCable/SM_ThinNetworkCable2.SM_ThinNetworkCable2"));
	CableSpline->SetStaticMesh(cableMesh);

	CableSpline->UpdateMesh();

	CableSpline->SetMobility(EComponentMobility::Type::Static);
}