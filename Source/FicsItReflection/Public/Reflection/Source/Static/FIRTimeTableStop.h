#pragma once

#include "CoreMinimal.h"
#include "FGRailroadTimeTable.h"
#include "FIRTrace.h"
#include "Buildables/FGBuildableRailroadStation.h"
#include "FIRTimeTableStop.generated.h"

/**
 * This is a mirror object of the FTimeTableStop
 * so it stores the referred station as a trace.
 * 
 * This is mainly used with in the kernel.
 */
USTRUCT()
struct FICSITREFLECTION_API FFIRTimeTableStop {
	GENERATED_BODY()
	
	UPROPERTY(SaveGame)
	FFIRTrace Station;

	UPROPERTY(SaveGame)
	FTrainDockingRuleSet RuleSet;

	operator FTimeTableStop() const {
		return FTimeTableStop{Cast<AFGBuildableRailroadStation>(*Station)->GetStationIdentifier(), RuleSet};
	}
};
