#pragma once

#include "FINNetworkConnectionComponent.h"
#include "FINNetworkComponent.h"
#include "Signals/FINSignalSender.h"
#include "Signals/FINSignalListener.h"
#include "FINDynamicStructHolder.h"
#include "FINAdvancedNetworkConnectionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINHandleSignal, const FFINDynamicStructHolder&, Signal, const FFINNetworkTrace&, Sender);

/**
 * This network connectionc component allows for cabled connections and additionally
 * it also allows for a basic implementation for a network component, signal sender and signal listener.
 */
UCLASS()
class FICSITNETWORKS_API UFINAdvancedNetworkConnectionComponent : public UFINNetworkConnectionComponent, public IFINNetworkComponent, public IFINSignalSender, public IFINSignalListener {
	GENERATED_BODY()

protected:
	/**
	 * The ID of this computer network component.
	 * Used to unqiuely identify it in the network.
	 * Gets automatically generated on begin play if it is nor already generated/saved.
	 */
	UPROPERTY(SaveGame)
	FGuid ID;

	/**
	 * The nick of this computer network component.
	 * Used to group components and give them an alias.
	 */
	UPROPERTY(SaveGame)
	FString Nick;

	/**
	 * Used to check if the ID is already generated.
	 */
	UPROPERTY(SaveGame)
	bool bIdCreated = false;
	
	/**
	 * The signal listeners listening to this component.
	 */
	UPROPERTY(SaveGame)
	TSet<FFINNetworkTrace> Listeners;

public:
	/**
	 * The object used as redirect object for network instancing of this component.
	 */
	UPROPERTY()
	UObject* RedirectionObject = nullptr;
	
	/**
	 * If set to true, connector will add it's owner as
	 */
	UPROPERTY(EditDefaultsOnly)
	bool bOuterAsRedirect = true;
	
	/**
	 * This event gets called if a signal ocures.
	 * It basically redirects the signal from the IFINSignalListener implementation.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Network|Connector")
	FFINHandleSignal OnNetworkSignal;

	UFINAdvancedNetworkConnectionComponent();
	~UFINAdvancedNetworkConnectionComponent();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Serialize(FArchive& Ar) override;
	// End AActor

	// Begin IFGSaveInterface
	bool ShouldSave_Implementation() const override;
	// End IFGSaveInterface

	// Begin IFINNetworkCircuitNode
	virtual void NotifyNetworkUpdate_Implementation(int Type, const TSet<UObject*>& Nodes) override;
	// End IFINNetworkCircuitNode

	// Begin IFINNetworkComponent
	virtual FGuid GetID_Implementation() const override;
	virtual FString GetNick_Implementation() const override;
	virtual void SetNick_Implementation(const FString& Nick) override;
	virtual bool HasNick_Implementation(const FString& Nick) override;
	virtual UObject* GetInstanceRedirect_Implementation() const override;
	virtual bool AccessPermitted_Implementation(FGuid ID) const override;
	// End IFINNetworkComponent

	// Begin IFINSignalSender
	virtual void AddListener_Implementation(FFINNetworkTrace Listener) override;
	virtual void RemoveListener_Implementation(FFINNetworkTrace Listener) override;
	virtual TSet<FFINNetworkTrace> GetListeners_Implementation() override;
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender

	// Begin IFINSignalListener
	virtual void HandleSignal(TSharedPtr<FFINSignal> Signal, FFINNetworkTrace Sender) override;
	// End IFINSignalListener

	/**
	 * This network signals gets emit when a network change occurs.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Signals")
	void netSig_NetworkUpdate(int changeType, const FString& changedComponent);
};