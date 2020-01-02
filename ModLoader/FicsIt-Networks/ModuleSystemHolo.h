#pragma once

#include <assets/BPInterface.h>
#include <util/Objects/FVector.h>

#include "../SatisfactorySDK/SDK.hpp"

#include "ModuleSystemPanel.h"

class AModuleSystemHolo : public SDK::AFGBuildableHologram {
public:
	UModuleSystemPanel* snapped;
	SML::Objects::FVector snappedLoc;
	int snappedRot;
	bool isValid;

	void construct();
	void destruct();

	SDK::AActor* constructFinal(SML::Objects::TArray<SDK::AActor*>& childs);
	bool isValidHit(const SDK::FHitResult& hit);
	void setHoloLocAndRot(SDK::FHitResult* hit);
	void checkValid();
	
private:
	bool checkSpace(SML::Objects::FVector min, SML::Objects::FVector max);
	SML::Objects::FVector getModuleSize();
};