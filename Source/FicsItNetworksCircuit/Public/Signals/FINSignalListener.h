#pragma once

#include "CoreMinimal.h"
#include "FINSignalData.h"
#include "FINSignalListener.generated.h"

/**
 * Allows the implementer to recieve network signals
 */
UINTERFACE()
class FICSITNETWORKSCIRCUIT_API UFINSignalListener : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKSCIRCUIT_API IFINSignalListener {
	GENERATED_IINTERFACE_BODY()
	
public:
	/**
	* Handle the given signal.
	*
	* @param	Signal	the signal you want to handle
	* @param	Sender	the sender of the signal
	*/
	virtual void HandleSignal(const FFINSignalData& Signal, const FFIRTrace& Sender) = 0;
};