#pragma once

#include "FINComputerModule.h"
#include "Network/FINNetworkCircuitNode.h"
#include "Network/FINNetworkComponent.h"
#include "Network/FINNetworkCustomType.h"
#include "Network/FINNetworkMessageInterface.h"
#include "Network/Signals/FINSignal.h"

#include "FINComputerNetworkCard.generated.h"

class AFINComputerCase;
UCLASS()
class AFINComputerNetworkCard : public AFINComputerModule, public IFINNetworkCircuitNode, public IFINNetworkComponent, public IFINNetworkMessageInterface, public IFINNetworkCustomType {
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
	virtual void HandleMessage(FGuid ID, FGuid Sender, FGuid Receiver, int Port, const TFINDynamicStruct<FFINParameterList>& Data) override;
	// End IFINNetworkMessageInterface

	// Begin IFINNetworkCustomType
	virtual FString GetCustomTypeName_Implementation() const override { return TEXT("NetworkCard"); }
	// End IFINNetworkCustomType

	UFUNCTION()
	void netFunc_open(int port);

	UFUNCTION()
	void netFunc_close(int port);

	UFUNCTION()
	void netFunc_closeAll();

	UFUNCTION()
	void netFunc_send(FString receiver, int port, FFINDynamicStructHolder args);

	UFUNCTION()
	void netFunc_broadcast(int port, FFINDynamicStructHolder args);
};

struct FFINNetworkCardArgChecker : public FFINValueReader {
	bool Fail = false;
	
	virtual void nil() override {};
	virtual void operator<<(FINBool B) override {};
	virtual void operator<<(FINInt Num) override {};
	virtual void operator<<(FINFloat Num) override {};
	virtual void operator<<(FINClass Class) override {};
	virtual void operator<<(const FINStr& Str) override {};
	virtual void operator<<(const FINObj& Obj) override;
	virtual void operator<<(const FINTrace& Obj) override;
	virtual void operator<<(const FINStruct& Struct) override;
	virtual void operator<<(const FINArray& Array) override;
};

USTRUCT()
struct FFINNetworkMessageSignal : public FFINSignal {
	GENERATED_BODY()

	FGuid Sender;
	int Port;
	TFINDynamicStruct<FFINParameterList> Data;

	FFINNetworkMessageSignal() = default;
	FFINNetworkMessageSignal(FGuid Sender, int Port, const TFINDynamicStruct<FFINParameterList>& Data);

	bool Serialize(FArchive& Ar);
	
	virtual int operator>>(FFINValueReader& reader) const override;
};

template<>
struct TStructOpsTypeTraits<FFINNetworkMessageSignal> : TStructOpsTypeTraitsBase2<FFINNetworkMessageSignal>
{
	enum
	{
		WithSerializer = true,
    };
};

inline bool operator<<(FArchive& Ar, FFINNetworkMessageSignal& Signal) {
	return Signal.Serialize(Ar);
}
