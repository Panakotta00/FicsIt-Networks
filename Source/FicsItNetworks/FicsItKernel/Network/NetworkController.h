
#pragma once

#include "CoreMinimal.h"
#include "FicsItNetworks/Network/FINAdvancedNetworkConnectionComponent.h"
#include "FicsItNetworks/Network/FINNetworkComponent.h"
#include "FicsItNetworks/Network/FINNetworkTrace.h"
#include "FicsItNetworks/Network/Signals/FINSignalData.h"
#include "NetworkController.generated.h"

/**
 * Allows to control and manage network connection of a system.
 * Also manages the network signals.
 */
UCLASS()
class FICSITNETWORKS_API UFINKernelNetworkController : public UObject, public IFGSaveInterface {
	GENERATED_BODY()
protected:
	FCriticalSection MutexSignals;
	TArray<TPair<FFINSignalData, FFINNetworkTrace>> SignalQueue;
	bool bLockSignalReceiving = false;

	/**
	 * Underlying Computer Network Component used for interacting with the network.
	 * Needs to be a Signal Sender and a Signal Listener which redirect the interaction of both to this.
	 */
	UPROPERTY()
	TScriptInterface<IFINNetworkComponent> Component = nullptr;
	
public:
	/**
	 * The maximum amount of signals the signal queue can hold
	 */
	uint32 MaxSignalCount = 1000;

	// Begin UObject
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	// End UObject

	// Begin IFGSaveInterface
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	// End IFGSaveInterface

	/**
	 * Sets the network component this controller is associated with.
	 *
	 * @param[in]	InNetworkComponent	the network component this controller uses
	 */
	void SetComponent(TScriptInterface<IFINNetworkComponent> InNetworkComponent);

	/**
	 * Returns the component used by the network controller to access the component network
	 * */
	TScriptInterface<IFINNetworkComponent> GetComponent() const;

	/**
	 * Should get called by the network component so the signal gets added to the signal queue
	 */
	void HandleSignal(const FFINSignalData& InSignal, const FFINNetworkTrace& InSender);

	/**
	 * pops a signal form the queue.
	 * returns nullptr if there is no signal left.
	 *
	 * @param[out]	OutSender	output-parameter for the sender of the signal
	 * @return	signal from the queue
	 */
	FFINSignalData PopSignal(FFINNetworkTrace& OutSender);

	/**
	 * pushes a signal to the queue.
	 * signal gets dropped if the queue is already full.
	 *
	 * @param[in]	InSignal	the signal you want to push
	 * @param[in]	InSender	the signal sender of signal you try to push
	 */
	void PushSignal(const FFINSignalData& InSignal, const FFINNetworkTrace& InSender);

	/**
	 * Removes all signals from the signal queue.
	 */
	void ClearSignals();

	/**
	 * gets the amount of signals in the queue
	 *
	 * @return	amount of signals
	 */
	uint64 GetSignalCount();

	/**
	 * tries to find a component with the given ID.
	 *
	 * @return	the component you searched for, nullptr if it was not able to find the component
	 */
	FFINNetworkTrace GetComponentByID(const FGuid& InID) const;

	/**
	 * returns the components in the network with the given nick.
	 */
	TSet<FFINNetworkTrace> GetComponentByNick(const FString& InNick) const;

	/**
	 * returns the components in the network with of the given type.
	 */
	TSet<FFINNetworkTrace> GetComponentByClass(UClass* InClass, bool bInRedirect) const;
};
