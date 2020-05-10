#pragma once

#include "CoreTypes.h"
#include "Misc/Guid.h"

struct FFINCustomVersion {
	enum Type {
		// Before any version changes were made
		BeforeCustomVersionWasAdded = 0,

		// FINComputerCase kernel system persistency
		KernelSystemPersistency,

        // -----<new versions can be added above this line>-------------------------------------------------
        VersionPlusOne,
        LatestVersion = VersionPlusOne - 1
	};
	
	static const FGuid GUID;
};
