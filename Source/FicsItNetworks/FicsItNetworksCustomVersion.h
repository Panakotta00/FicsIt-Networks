#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "FicsItNetworksCustomVersion.generated.h"

UENUM(BlueprintType)
enum EFINCustomVersion {
	// Before any version changes were made
	FINBeforeCustomVersionWasAdded = 0,

    // Signal Storage / Trace / "Parameter List" overhaul
    FINSignalStorage,

	// Codeable Splitter Attachment Fixes
	FINCodeableSplitterAttachmentFixes,

    // -----<new versions can be added above this line>-------------------------------------------------
    FINVersionPlusOne,
    FINLatestVersion = FINVersionPlusOne - 1
};

struct FFINCustomVersion {
	static const FGuid GUID;
};
