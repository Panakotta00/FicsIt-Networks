#pragma once

#include "FINSignal.h"
#include "Network/FINAnyNetworkValue.h"
#include "Network/FINNetworkTrace.h"
#include "FINSmartSignal.generated.h"

/**
 * A signal container which uses a c++ variadic parameters list
 * to store the signal parameters.
 */
USTRUCT()
struct FFINSmartSignal : public FFINSignal {
	GENERATED_BODY()
	
protected:
	TArray<FFINAnyNetworkValue> Args;

public:
	FFINSmartSignal();
	FFINSmartSignal(FString name, const TArray<FFINAnyNetworkValue>& args) : FFINSignal(name), Args(args) {}
	
	template<typename... Ts>
	FFINSmartSignal(FString signalName, Ts&&... args) : FFINSmartSignal(signalName, {FFINAnyNetworkValue(args)...}) {}

	bool Serialize(FArchive& Ar);
	
	// Begin FFINSignal
	virtual int operator>>(FFINValueReader& reader) const override;
	// End FFINSignal
};

template<>
struct TStructOpsTypeTraits<FFINSmartSignal> : TStructOpsTypeTraitsBase2<FFINSmartSignal>
{
	enum
	{
		WithSerializer = true,
    };
};
