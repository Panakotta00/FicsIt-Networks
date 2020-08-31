#pragma once

#include "Interface.h"
#include "WeakInterfacePtr.h"

#include "FINNetworkCircuitNode.generated.h"

class AFINNetworkCircuit;

/**
 * Everything that can be connected to a network circuit,
 * is required to implement this interface.
 */
UINTERFACE(Blueprintable)
class UFINNetworkCircuitNode : public UInterface {
	GENERATED_BODY()
};

class IFINNetworkCircuitNode {
	GENERATED_BODY()

public:
	/**
	 * Returns a array of connected nodes.
	 * These nodes are used to refer to "neighbours" in the network and so, actually create a network of nodes.
	 * Contains only the direct neighbours of the node.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
    TSet<UObject*> GetConnected() const;

	/**
	* Returns the connected network circuit of this node.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
    AFINNetworkCircuit* GetCircuit() const;

	/**
	* Sets the connected network circuit of this node.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
    void SetCircuit(AFINNetworkCircuit* Circuit);

	/**
	* This functions gets executed when a change in the computer network circuit occured.
	* Like adding or removing a new node.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
    void NotifyNetworkUpdate(int32 Type, const TSet<UObject*>& Nodes);
};