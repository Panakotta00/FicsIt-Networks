#pragma once

#include "FINNetworkConnectionComponent.h"
#include "FINNetworkComponent.h"
#include "Signals/FINSignalSender.h"
#include "Signals/FINSignalListener.h"
#include "FINDynamicStructHolder.h"
#include "FINNetworkMessageInterface.h"

#include "FINAdvancedNetworkConnectionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINHandleSignal, const FFINSignalData&, Signal, const FFINNetworkTrace&, Sender);
DECLARE_MULTICAST_DELEGATE_FiveParams(FFINHandleNetworkMessage, FGuid, FGuid, FGuid, int, const TArray<FFINAnyNetworkValue>&);
DECLARE_DELEGATE_RetVal(bool, FFINIsNetworkRouter);
DECLARE_DELEGATE_RetVal_OneParam(bool, FFINIsNetworkPortOpen, int);

/**
 * This network connectionc component allows for cabled connections and additionally
 * it also allows for a basic implementation for a network component, signal sender and signal listener.
 */
UCLASS()
class FICSITNETWORKS_API UFINAdvancedNetworkConnectionComponent : public UFINNetworkConnectionComponent, public IFINNetworkComponent, public IFINSignalSender, public IFINSignalListener, public IFINNetworkMessageInterface {
	GENERATED_BODY()

protected:
	/**
	 * The ID of this computer network component.
	 * Used to unqiuely identify it in the network.
	 * Gets automatically generated on begin play if it is nor already generated/saved.
	 */
	UPROPERTY(SaveGame, Replicated)
	FGuid ID;

	/**
	 * The nick of this computer network component.
	 * Used to group components and give them an alias.
	 */
	UPROPERTY(SaveGame, Replicated)
	FString Nick;

	/**
	 * Used to check if the ID is already generated.
	 */
	UPROPERTY(SaveGame)
	bool bIdCreated = false;
	
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

	FFINHandleNetworkMessage OnNetworkMessageRecieved;
	FFINIsNetworkRouter OnIsNetworkRouter;
	FFINIsNetworkPortOpen OnIsNetworkPortOpen;

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
	virtual UObject* GetSignalSenderOverride_Implementation() override;
	// End IFINSignalSender

	// Begin IFINSignalListener
	virtual void HandleSignal(const FFINSignalData& Signal, const FFINNetworkTrace& Sender) override;
	// End IFINSignalListener

	// Begin IFINNetworkMessageInterface
	virtual bool IsPortOpen(int Port) override;
	virtual void HandleMessage(FGuid ID, FGuid Sender, FGuid Receiver, int Port, const TArray<FFINAnyNetworkValue>& Data) override;
	virtual bool IsNetworkMessageRouter() const override;
	// End IFINNetworkMessageInterface

	/**
	 * This network signals gets emit when a network change occurs.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Signals")
	void netSig_NetworkUpdate(int changeType, const FString& changedComponent);
};