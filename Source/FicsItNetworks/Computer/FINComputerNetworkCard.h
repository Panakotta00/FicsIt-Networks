#pragma once

#include "FINComputerModule.h"
#include "Network/FINNetworkComponent.h"
#include "Network/FINNetworkMessageInterface.h"
#include "Network/Signals/FINSignal.h"

#include "FINComputerNetworkCard.generated.h"

class AFINComputerCase;
UCLASS()
class AFINComputerNetworkCard : public AFINComputerModule, public IFINNetworkComponent, public IFINNetworkMessageInterface {
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
	 * The only one connected network component to this module.
	 * Only used for building the computer network.
	 */
	UPROPERTY()
	UObject* ConnectedComponent = nullptr;

	/**
	* The computer network circuit this component is connected to.
	*/
	UPROPERTY()
	UFINNetworkCircuit* Circuit = nullptr;
	
	// Begin AActor
	virtual void BeginPlay() override;
	// End AActor

	// Begin IFINNetworkComponent
	virtual FGuid GetID_Implementation() const override;
	virtual FString GetNick_Implementation() const override;
	virtual void SetNick_Implementation(const FString& nick) override;
	virtual bool HasNick_Implementation(const FString& nick) override;
	virtual TSet<UObject*> GetMerged_Implementation() const override;
	virtual TSet<UObject*> GetConnected_Implementation() const override;
	virtual FFINNetworkTrace FindComponent_Implementation(FGuid id) const override;
	virtual UFINNetworkCircuit* GetCircuit_Implementation() const override;
	virtual void SetCircuit_Implementation(UFINNetworkCircuit* circuit) override;
	virtual void NotifyNetworkUpdate_Implementation(int type, const TSet<UObject*>& nodes) override;
	// End IFINNetworkComponent

	// Begin IFINNetworkMessageInterface
	virtual bool IsPortOpen(int Port) override;
	virtual void HandleMessage(FFINNetworkTrace Sender, int Port, const TFINDynamicStruct<FFINParameterList>& Data) override;
	// End IFINNetworkMessageInterface

	UFUNCTION()
	void netFunc_open(int port);

	UFUNCTION()
	void netFunc_close(int port);

	UFUNCTION()
	void netFunc_closeAll();

	UFUNCTION()
	void netFunc_send(FFINNetworkTrace reciever, int port, FFINDynamicStructHolder args);

	UFUNCTION()
	void netFunc_broadcast(int port, FFINDynamicStructHolder args);
};

USTRUCT()
struct FFINNetworkMessageSignal : public FFINSignal {
	GENERATED_BODY()
	
	int Port;
	TFINDynamicStruct<FFINParameterList> Data;

	FFINNetworkMessageSignal() = default;
	FFINNetworkMessageSignal(int Port, const TFINDynamicStruct<FFINParameterList>& Data);

	bool Serialize(FArchive& Ar);
	
	virtual int operator>>(FFINValueReader& reader) const override;
	virtual UScriptStruct* GetStruct() const override { return StaticStruct(); };
};

inline bool operator<<(FArchive& Ar, FFINNetworkMessageSignal& Signal) {
	return Signal.Serialize(Ar);
}
