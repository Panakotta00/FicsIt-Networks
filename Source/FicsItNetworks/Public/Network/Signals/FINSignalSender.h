#pragma once

#include "CoreMinimal.h"
#include "FIRTrace.h"
#include "FINSignalSender.generated.h"

/**
 * Allows the implementer to send signals by f.e. managing the listeners.
 */
UINTERFACE(Blueprintable)
class FICSITNETWORKS_API UFINSignalSender : public UInterface  {
	GENERATED_BODY()
};
	
class FICSITNETWORKS_API IFINSignalSender {
	GENERATED_IINTERFACE_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Signals|Sender")
	UObject* GetSignalSenderOverride();
};
