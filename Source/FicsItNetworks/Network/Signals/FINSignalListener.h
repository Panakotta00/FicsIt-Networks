#pragma once

#include "CoreMinimal.h"
#include "Interface.h"
#include "FINSignal.h"
#include "Network/FINNetworkTrace.h"

#include "FicsItKernel/Network/SignalListener.h"

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
	
private:
	class SignalListenerImpl : public FicsItKernel::Network::SignalListener {
	private:
		IFINSignalListener* parent = nullptr;

	public:
		SignalListenerImpl(IFINSignalListener* parent);

		virtual void handleSignal(std::shared_ptr<FicsItKernel::Network::Signal> signal, FicsItKernel::Network::NetworkTrace sender) override;
	};

	std::shared_ptr<FicsItKernel::Network::SignalListener> impl;

public:
	IFINSignalListener();

	operator std::shared_ptr<FicsItKernel::Network::SignalListener>();

	/**
	* Handle the given signal.
	*
	* @param	signal	the signal you want to handle
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Network|Signal|Listener")
	void HandleSignal(FFINSignal signal, FFINNetworkTrace sender);
};