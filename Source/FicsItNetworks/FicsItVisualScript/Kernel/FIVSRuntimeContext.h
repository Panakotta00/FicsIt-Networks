#pragma once

#include "FicsItNetworks/FicsItKernel/FicsItKernel.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSGraph.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSNode.h"
#include "FicsItNetworks/Network/FINAnyNetworkValue.h"
#include "FIVSRuntimeContext.generated.h"

USTRUCT()
struct FFIVSRuntimeContext {
	GENERATED_BODY()
private:
	UPROPERTY()
	TMap<UFIVSPin*, FFINAnyNetworkValue> PinValues;

	UPROPERTY()
	UFINKernelSystem* KernelContext = nullptr;

	UPROPERTY()
	UFIVSGraph* Script = nullptr;
	
public:
	FFIVSRuntimeContext() = default;
	FFIVSRuntimeContext(UFIVSGraph* InScript, UFINKernelSystem* InKernelContext) {
		Script = InScript;
		KernelContext = InKernelContext;
	}
	
	UFIVSGraph* GetScript() { return Script; }
	UFINKernelSystem* GetKernelContext() { return KernelContext; }
	
	/**
	 * This function tries to find a value that is stored for the pin, like if a pin got evaluated,
	 * this function has to be used to get the value of the evaluation.
	 * If no value is set directly on the pin, it will try to follow the pin connection and get the value of the
	 * connected pin if it is available, otherwise it returns nil.
	 * If no value is set directly and there are no connections to other pins, returns the literal value of the pin.
	 */
	FFINAnyNetworkValue GetValue(UFIVSPin* InPin) {
		FFINAnyNetworkValue* Value = PinValues.Find(InPin);
		if (Value) return *Value;
		if (InPin->GetConnections().Num() < 1) {
			return InPin->GetLiteral();
		} else {
			UFIVSPin* Source = InPin->FindConnected();
			if (Source) Value = PinValues.Find(Source);
			if (Value) return *Value;
			return FFINAnyNetworkValue(InPin->GetPinDataType());
		}
	}

	/**
	 * This function can be used to define the value that a data-output-pin currently returns
	 */
	void SetValue(UFIVSPin* InPin, FFINAnyNetworkValue Value) {
		PinValues.FindOrAdd(InPin) = FINCastNetworkValue(Value, InPin->GetPinDataType());
	}
};
