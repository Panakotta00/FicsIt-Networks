#pragma once

#include "CoreMinimal.h"
#include "FINNetworkCircuitNode.h"
#include "FINNetworkComponent.h"
#include "Network/FINNetworkTrace.h"
#include "FINNetworkCircuit.generated.h"

class UFINAdvancedNetworkConnectionComponent;

/**
 * Manages and caches a computer network circuit.
 * When changes occur in the network, also sends signals to the componentes accordingly.
 */
UCLASS()
class FICSITNETWORKS_API UFINNetworkCircuit : public UObject {
	GENERATED_BODY()
	
	friend UFINAdvancedNetworkConnectionComponent;

protected:
	UPROPERTY(Replicated)
	TSet<TSoftObjectPtr<UObject>> Nodes;

	void AddNodeRecursive(TSet<TScriptInterface<IFINNetworkCircuitNode>>& Added, TScriptInterface<IFINNetworkCircuitNode> Add);

public:
	UFINNetworkCircuit();
	~UFINNetworkCircuit();

	/**
	 * Adds the given circuit to this circuit.
	 * Causes correct update signals for the components.
	 */
	UFINNetworkCircuit* operator+(UFINNetworkCircuit* Circuit);
	
	/**
	 * Regenerates the node cache based on the given start component
	 */
	void Recalculate(const TScriptInterface<IFINNetworkCircuitNode>& Node);

	/**
	 * Returns if the given node is part of the circuit based on the circuit cache
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	bool HasNode(const TScriptInterface<IFINNetworkCircuitNode>& Node);

	/**
	 * Trys to find the component with the given address/is in the circuit cache.
	 *
	 * @param[in]	ID			the id of the component you try to find
	 * @param[in]	Requester	the reference to the requesting component, if set, enables permitted access filtering
	 * @return	the found component, if it cant find it nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	TScriptInterface<IFINNetworkComponent> FindComponent(const FGuid& ID, const TScriptInterface<IFINNetworkComponent>& Requester);

	/**
	 * Trys to find components with the given nick in the circuit cache.
	 *
	 * @param[in]	Nick		the nick query you use to search for the components
	 * @param[in]	Requester	the reference to the requesting component, if set, enables permitted access filtering
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	TSet<UObject*> FindComponentsByNick(const FString& Nick, const TScriptInterface<IFINNetworkComponent>& Requester);

	/**
	 * Returns all components in the circuit cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	TSet<UObject*> GetComponents();

	/**
	 * Checks if the given node is part of the circuit started by the given node based on the circuit connections.
	 * @warning	slow! You should use HasNode since it uses the cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
    static bool IsNodeConnected(const TScriptInterface<IFINNetworkCircuitNode>& Start, const TScriptInterface<IFINNetworkCircuitNode>& Node);

	/**
	 * Updates the circuits of node A and B after node B got removed
	 * from the circuit of node A
	 * by creating the new circuit, recalculating the circuits etc.
	 * Should get called after the the nodes got disconnected
	 *
	 * @param[in]	A	the node whichs circuit should remove node B
	 * @param[in]	B	the node which sould get removed
	 */
	UFUNCTION()
	static void DisconnectNodes(const TScriptInterface<IFINNetworkCircuitNode>& A, const TScriptInterface<IFINNetworkCircuitNode>& B);

	/**
	 * Updates the circuits of node A and B after they got connected
	 * by merging and recalculating the circuits.
	 * Should get called after the nodes got connected.
	 *
	 * @param[in]	A	the first component
	 * @param[in]	B	the second component
	 */
	UFUNCTION()
	static void ConnectNodes(const TScriptInterface<IFINNetworkCircuitNode>& A, const TScriptInterface<IFINNetworkCircuitNode>& B);

private:
	static bool IsNodeConnected_Internal(const TScriptInterface<IFINNetworkCircuitNode>& Self, const TScriptInterface<IFINNetworkCircuitNode>& Node, TSet<UObject*>& Searched);
};