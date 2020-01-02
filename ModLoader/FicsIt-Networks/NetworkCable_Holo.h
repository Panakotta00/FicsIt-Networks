#pragma once

#include <util/Objects/TArray.h>
#include <util/Objects/FVector.h>
#include <SDK.hpp>
#include <assets/BPInterface.h>
#include "NetworkConnector.h"

struct SnappedInfo {
	bool v = false;
	bool isConnector = false;
	void* ptr = nullptr;
	SDK::AActor* actor = nullptr;
	SML::Objects::FVector pos;

	inline UNetworkConnector* c() { return (UNetworkConnector*)ptr; }
	inline SDK::AFGBuildableFactory* f() { return (SDK::AFGBuildableFactory*)ptr; }
};

class ANetworkCable_Holo : public SDK::AFGBuildableHologram {
public:
	SDK::USplineMeshComponent* cable;
	SDK::UStaticMeshComponent* con;
	SnappedInfo snapped;
	SnappedInfo oldSnapped;
	SnappedInfo from;

	bool multiPlacement();
	SDK::AActor* constructFinal(SML::Objects::TArray<SDK::AActor*>& childs);
	SML::Objects::TArray<SDK::FItemAmount> getCost(bool includeChildren);
	bool isValidHit(const SDK::FHitResult& hit);
	bool isSnappedValid();
	void setHoloLocAndRot(SDK::FHitResult* hit);
	void onInvalidHolo();

	void onBeginSnap(SnappedInfo a, bool isValid);
	void onEndSnap(SnappedInfo a);
	void updateMeshes();

	bool isValid();

	static void constructOI(SML::Paks::FObjectInitializer& oi);
	void construct();
	void destruct();
};