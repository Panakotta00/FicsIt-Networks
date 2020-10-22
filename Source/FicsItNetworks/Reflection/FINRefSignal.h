#pragma once

#include "FINProperty.h"
#include "Network/Signals/FINSignal.h"
#include "Network/Signals/FINSignalSubsystem.h"


#include "FINRefSignal.generated.h"

UCLASS(BlueprintType)
class UFINRefSignal : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	FText Description;
	UPROPERTY()
	FString InternalName = TEXT("UnknownSignal");
	UPROPERTY()
	FText DisplayName = FText::FromString(TEXT("Unknown Signal"));
	UPROPERTY()
	TArray<UFINProperty*> Parameters;
	
	/**
	 * Returns the description of this signal
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FText GetDescription() { return Description; }
	
	/**
	 * Returns a more cryptic name of the signal, used mainly for internal reference
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FString GetInternalName() { return InternalName; }

	/**
	 * Returns a human readable name of the signal, mainly used for UI
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual FText GetDisplayName() { return DisplayName; }

	/**
	 * Returns the parameters of the signal
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TArray<UFINProperty*> GetParameters() { return Parameters; }

	/**
	 * Triggers the Signal
	 */
	virtual void Trigger(UObject* Context, TFINDynamicStruct<FFINSignal> Signal) {
		AFINSignalSubsystem::GetSignalSubsystem(Context)->BroadcastSignal(Context, Signal);
	}
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void Trigger(UObject* Context, FFINDynamicStructHolder Signal) {
		Trigger(Context, TFINDynamicStruct<FFINSignal>(Signal));
	}
};
