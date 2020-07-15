#pragma once

#include "CoreMinimal.h"
#include "Interface.h"
#include "Function.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "FINSignal.h"
#include "Network/FINNetworkTrace.h"
#include "FINSignalListener.h"

#include "FicsItKernel/Network/Signal.h"

#include <memory>
#include <string>

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
	void AddListener(FFINNetworkTrace listener);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Signals|Sender")
	void RemoveListener(FFINNetworkTrace listener);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Signals|Sender")
	TSet<FFINNetworkTrace> GetListeners();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Signals|Sender")
	UObject* GetSignalSenderOverride();
};

UCLASS()
class FICSITNETWORKS_API UFINSignalUtility : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

public:
	/**
	 * This functions manipulates the signal UFunctions of the given class so the signals get actually send to the listeners.
	 * This function should get called by the classes on their first construct.
	 * Multiple calling of this function for the same class is allowed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Signals|Sender")
	static void SetupSender(UClass* signalSender);

	class BPSignal : public FicsItKernel::Network::Signal {
	private:
		UFunction* func = nullptr;
		void* data = nullptr;

	public:
		BPSignal(std::string name, UFunction* func, void* data);
		BPSignal(FArchive& Ar);
		
		virtual int operator>>(FicsItKernel::Network::SignalReader& reader) const override;

		virtual std::string getTypeName() override;

		virtual void Serialize(FArchive& Ar) override;
	};
};