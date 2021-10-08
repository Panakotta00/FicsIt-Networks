#pragma once

#include "FicsItNetworks/FicsItVisualScript/Script/FIVSNode.h"
#include "FicsItNetworks/Network/FINAnyNetworkValue.h"
#include "FIVSRuntimeContext.generated.h"

USTRUCT()
struct FFIVSRuntimeContext {
	GENERATED_BODY()
private:
	UPROPERTY()
	TMap<UFIVSPin*, FFINAnyNetworkValue> PinValues;
	
public:
	/**
	 * This function trys to find a value that is stored for the pin, like if a pin got evaluated,
	 * this function has to be used to get the value of the evaluation.
	 */
	FFINAnyNetworkValue GetValue(UFIVSPin* InPin) {
		return PinValues[InPin];
	}
};
