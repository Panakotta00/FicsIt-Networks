#pragma once

#include "CoreMinimal.h"
#include "FINSignalSender.generated.h"

/**
 * Allows the implementer to send signals by f.e. managing the listeners.
 */
UINTERFACE(Blueprintable)
class FICSITNETWORKSCIRCUIT_API UFINSignalSender : public UInterface  {
	GENERATED_BODY()
};
	
class FICSITNETWORKSCIRCUIT_API IFINSignalSender {
	GENERATED_IINTERFACE_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Signals|Sender")
	UObject* GetSignalSenderOverride();
};
