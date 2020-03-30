#pragma once

#include "CoreMinimal.h"
#include "FicsItKernel/Network/Signal.h"
#include <memory>
#include "FINSignal.generated.h"

/**
 * Holds information like the paramters of signal
 */
USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINSignal {
	GENERATED_BODY()

private:
	std::shared_ptr<FicsItKernel::Network::Signal> signal;

public:
	FFINSignal();
	FFINSignal(const std::shared_ptr<FicsItKernel::Network::Signal>& signal);

	operator std::shared_ptr<FicsItKernel::Network::Signal>();
};