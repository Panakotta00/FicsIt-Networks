#pragma once

#include "FicsItNetworks/Network/FINNetworkTrace.h"
#include "FGRailroadTimeTable.h"
#include "Buildables/FGBuildableRailroadStation.h"
#include "FINTimeTableStop.generated.h"

/**
 * This is a mirror object of the FTimeTableStop
 * so it stores the referred station as a trace.
 * 
 * This is mainly used with in the kernel.
 */
USTRUCT()
struct FICSITNETWORKS_API FFINTimeTableStop {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FFINNetworkTrace Station;

	UPROPERTY(SaveGame)
	float Duration = 0.0f;

	operator FTimeTableStop() const {
		return FTimeTableStop{Cast<AFGBuildableRailroadStation>(*Station)->GetStationIdentifier(), Duration};
	}
};
