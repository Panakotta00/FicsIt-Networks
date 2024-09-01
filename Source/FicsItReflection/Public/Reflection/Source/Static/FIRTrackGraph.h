#pragma once

#include "CoreMinimal.h"
#include "FIRTrace.h"
#include "FIRTrackGraph.generated.h"

/**
 * Stores a track graph ID and a network trace to object
 * that refers to the track graph (like train, railroad vehicle, track, station).
 * This trace allows so make sure that someone has access to the graph.
 * This struct is mostly used for the usage in the kernel.
 */
USTRUCT()
struct FICSITREFLECTION_API FFIRTrackGraph {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FFIRTrace Trace;

	UPROPERTY(SaveGame)
	int TrackID;

	static int GetTrackID(UObject* obj);

	bool IsValid();
};
