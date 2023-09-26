#pragma once

#include "FINProperty.h"
#include "FicsItNetworksModule.h"
#include "Network/Signals/FINSignalSubsystem.h"
#include "FINSignal.generated.h"

UENUM()
enum EFINSignalFlags {
	FIN_Signal_None			= 0,
	FIN_Signal_StaticSource	= 0b1,
};

ENUM_CLASS_FLAGS(EFINSignalFlags)

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINSignal : public UFINBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	bool bIsVarArgs = false;
	UPROPERTY()
	TArray<UFINProperty*> Parameters;

	EFINSignalFlags SignalFlags = FIN_Signal_None;

	/**
	 * Returns the signal flags of this struct
	 */
	virtual EFINSignalFlags GetSignalFlags() const { return SignalFlags; }
	
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
		AFINSignalSubsystem* SubSys = AFINSignalSubsystem::GetSignalSubsystem(Context);
		if (!SubSys) {
			UE_LOG(LogFicsItNetworks, Error, TEXT("Unable to get signal subsystem for executing signal '%s'"), *GetInternalName())
			return;
		}
		SubSys->BroadcastSignal(Context, FFINSignalData(this, Data));
	}
};
