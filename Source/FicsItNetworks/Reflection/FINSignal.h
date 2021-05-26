#pragma once

#include "FINProperty.h"
#include "FicsItNetworks/Network/Signals/FINSignalSubsystem.h"
#include "FINSignal.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINSignal : public UFINBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	bool bIsVarArgs = false;
	UPROPERTY()
	TArray<UFINProperty*> Parameters;
	
	/**
	 * Returns the parameters of the signal
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINProperty*> GetParameters() const { return Parameters; }

	/**
	 * Returns if the signal sends a variable amount of arguments
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual bool IsVarArgs() const { return bIsVarArgs; }

	/**
	 * Triggers the Signal
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual void Trigger(UObject* Context, const TArray<FFINAnyNetworkValue>& Data) {
		AFINSignalSubsystem::GetSignalSubsystem(Context)->BroadcastSignal(Context, FFINSignalData(this, Data));
	}
};
