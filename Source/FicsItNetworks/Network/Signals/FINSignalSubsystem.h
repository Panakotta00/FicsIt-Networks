#pragma once

#include "FGSubsystem.h"
#include "FINSignal.h"
#include "Network/FINDynamicStructHolder.h"

#include "FINSignalSubsystem.generated.h"

USTRUCT()
struct FFINSignalListeners {
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FFINNetworkTrace> Listeners;
};

UCLASS(BlueprintType)
class AFINSignalSubsystem : public AFGSubsystem, public IFGSaveInterface {
	GENERATED_BODY()
private:
	/**
	 * Map of sender objects to array of receiver traces, traces point from sender to receiver
	 */
	UPROPERTY(SaveGame)
	TMap<UObject*, FFINSignalListeners> Listeners;
public:
	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface
	
	/**
	* Gets the loaded signal subsystem in the given world.
	*
	* @param[in]	WorldContext	the world context from were to load the signal subsystem.
	*/
	UFUNCTION(BlueprintCallable, Category = "Network|Signals", meta = (WorldContext = "WorldContext"))
    static AFINSignalSubsystem* GetSignalSubsystem(UObject* WorldContext);

	/**
	 * Distributes the given signal to all listeners listening to the given object
	 */
	void BroadcastSignal(UObject* Sender, TFINDynamicStruct<FFINSignal> Signal);

	/**
	 * Adds the given listener to the listener list of the given sender
	 * the Receiver Trace should point from sender to the receiver
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Signals")
	void Listen(UObject* Sender, const FFINNetworkTrace& Receiver);

	/**
	 * Removes the given listener from the listener list of the given sender
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Signals")
	void Ignore(UObject* Sender, UObject* Receiver);

	/**
	 * Removes the given listener form all senders
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Signals")
	void IgnoreAll(UObject* Receiver);
};
