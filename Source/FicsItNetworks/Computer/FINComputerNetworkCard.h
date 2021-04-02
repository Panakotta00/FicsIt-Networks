#pragma once

#include "FINComputerModule.h"
#include "FicsItNetworks/Network/FINNetworkCircuitNode.h"
#include "FicsItNetworks/Network/FINNetworkComponent.h"
#include "FicsItNetworks/Network/FINNetworkMessageInterface.h"
#include "FicsItNetworks/Network/Signals/FINSignalData.h"

#include "FINComputerNetworkCard.generated.h"

class AFINComputerCase;
UCLASS()
class FICSITNETWORKS_API AFINComputerNetworkCard : public AFINComputerModule, public IFINNetworkCircuitNode, public IFINNetworkComponent, public IFINNetworkMessageInterface {
	GENERATED_BODY()
public:
	/**
	 * List of open ports the network card is currently listening to.
	 */
	UPROPERTY(SaveGame)
	TSet<int> OpenPorts;
	
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

	/**
	 * The only one connected network component to this module.
	 * Only used for building the computer network.
	 */
	UPROPERTY()
	UObject* ConnectedComponent = nullptr;

	/**
	* The computer network circuit this component is connected to.
	*/
	UPROPERTY()
	AFINNetworkCircuit* Circuit = nullptr;

	/**
	 * Set of ID of already handled network messages since last tick.
	 */
	UPROPERTY()
	TSet<FGuid> HandledMessages;
	FCriticalSection HandledMessagesMutex;

	AFINComputerNetworkCard();
	
	// Begin AActor
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	// End AActor

	// Begin IFINNetworkCircuitNode
	virtual TSet<UObject*> GetConnected_Implementation() const override;
	virtual AFINNetworkCircuit* GetCircuit_Implementation() const override;
	virtual void SetCircuit_Implementation(AFINNetworkCircuit* Circuit) override;
	virtual void NotifyNetworkUpdate_Implementation(int Type, const TSet<UObject*>& Nodes) override;
	// End IFINNetworkCircuitNodes
	
	// Begin IFINNetworkComponent
	virtual FGuid GetID_Implementation() const override;
	virtual FString GetNick_Implementation() const override;
	virtual void SetNick_Implementation(const FString& Nick) override;
	virtual bool HasNick_Implementation(const FString& Nick) override;
	virtual UObject* GetInstanceRedirect_Implementation() const override;
	virtual bool AccessPermitted_Implementation(FGuid ID) const override;
	// End IFINNetworkComponent

	// Begin IFINNetworkMessageInterface
	virtual bool IsPortOpen(int Port) override;
	virtual void HandleMessage(FGuid InID, FGuid Sender, FGuid Receiver, int Port, const TArray<FFINAnyNetworkValue>& Data) override;
	// End IFINNetworkMessageInterface

	static bool CheckNetMessageData(const TArray<FFINAnyNetworkValue>& Data);

	UFUNCTION()
    void netClass_Meta(FString& InternalName, FText& DisplayName) {
		InternalName = TEXT("NetworkCard");
		DisplayName = FText::FromString(TEXT("Network Card"));
	}
	
	UFUNCTION()
	void netFunc_open(int port);
	UFUNCTION()
    void netFuncMeta_open(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "open";
		DisplayName = FText::FromString("Open Port");
		Description = FText::FromString("Opens the given port so the network card is able to receive network messages on the given port.");
		ParameterInternalNames.Add("port");
		ParameterDisplayNames.Add(FText::FromString("Port"));
		ParameterDescriptions.Add(FText::FromString("The port you want to open."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_close(int port);
	UFUNCTION()
    void netFuncMeta_close(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "close";
		DisplayName = FText::FromString("Close Port");
		Description = FText::FromString("Closes the given port so the network card wont receive network messages on the given port.");
		ParameterInternalNames.Add("port");
		ParameterDisplayNames.Add(FText::FromString("Port"));
		ParameterDescriptions.Add(FText::FromString("The port you want to close."));
		Runtime = 1;
	}
	
	UFUNCTION()
	void netFunc_closeAll();
	UFUNCTION()
    void netFuncMeta_closeAll(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "closeAll";
		DisplayName = FText::FromString("Close All Ports");
		Description = FText::FromString("Closes all ports of the network card so no further messages are able to get received");
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_send(FString receiver, int port, const TArray<FFINAnyNetworkValue>& varargs);
	UFUNCTION()
	void netFuncMeta_send(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "send";
		DisplayName = FText::FromString("Send Message");
		Description = FText::FromString("Sends a network message to the receiver with the given address on the given port. The data you want to add can be passed as additional parameters. Max amount of such parameters is 7 and they can only be nil, booleans, numbers and strings.");
		ParameterInternalNames.Add("receiver");
		ParameterDisplayNames.Add(FText::FromString("Receiver"));
		ParameterDescriptions.Add(FText::FromString("The component ID as string of the component you want to send the network message to."));
		ParameterInternalNames.Add("port");
		ParameterDisplayNames.Add(FText::FromString("Port"));
		ParameterDescriptions.Add(FText::FromString("The port on which the network message should get sent. For outgoing network messages a port does not need to be opened."));
		Runtime = 1;
	}

	UFUNCTION()
	void netFunc_broadcast(int port, const TArray<FFINAnyNetworkValue>& varargs);
	UFUNCTION()
	void netFuncMeta_broadcast(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "broadcast";
		DisplayName = FText::FromString("Broadcast Message");
		Description = FText::FromString("Sends a network message to all components in the network message network (including networks sepperated by network routers) on the given port. The data you want to add can be passed as additional parameters. Max amount of such parameters is 7 and they can only be nil, booleans, numbers and strings.");
		ParameterInternalNames.Add("port");
		ParameterDisplayNames.Add(FText::FromString("Port"));
		ParameterDescriptions.Add(FText::FromString("The port on which the network message should get sent. For outgoing network messages a port does not need to be opened."));
		Runtime = 1;
	}

	UFUNCTION()
	void netSig_NetworkMessage(const FString& sender, int port, const TArray<FFINAnyNetworkValue>& varargs) {}
	UFUNCTION()
    void netSigMeta_NetworkMessage(FString& InternalName, FText& DisplayName, FText& Description, TArray<FString>& ParameterInternalNames, TArray<FText>& ParameterDisplayNames, TArray<FText>& ParameterDescriptions, int32& Runtime) {
		InternalName = "NetworkMessage";
		DisplayName = FText::FromString("Network Message");
		Description = FText::FromString("Triggers when the network card receives a network message on one of its opened ports. The additional arguments are the data that is contained within the network message.");
		ParameterInternalNames.Add("sender");
		ParameterDisplayNames.Add(FText::FromString("Sender"));
		ParameterDescriptions.Add(FText::FromString("The component id of the sender of the network message."));
		ParameterInternalNames.Add("port");
		ParameterDisplayNames.Add(FText::FromString("Port"));
		ParameterDescriptions.Add(FText::FromString("The port on which the network message got sent."));
		Runtime = 1;
	}
};
