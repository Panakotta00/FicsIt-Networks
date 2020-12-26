#pragma once

#include "CoreMinimal.h"
#include "Interface.h"
#include "FINSignalData.h"
#include "Network/FINNetworkTrace.h"

#include "FINSignalListener.generated.h"

/**
 * Allows the implementer to recieve network signals
 */
UINTERFACE()
class FICSITNETWORKS_API UFINSignalListener : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKS_API IFINSignalListener {
	GENERATED_IINTERFACE_BODY()
	
public:
	/**
	* Handle the given signal.
	*
	* @param	Signal	the signal you want to handle
	* @param	Sender	the sender of the signal
	*/
	virtual void HandleSignal(const FFINSignalData& Signal, const FFINNetworkTrace& Sender) = 0;
};