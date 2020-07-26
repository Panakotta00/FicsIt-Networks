#pragma once

#include "CoreMinimal.h"
#include "Network/FINNetworkTrace.h"
#include "FINNetworkCircuit.generated.h"

class UFINNetworkConnector;

/**
 * Manages and caches a computer network circuit.
 * When changes occur in the network, also sends signals to the componentes accordingly.
 */
UCLASS()
class FICSITNETWORKS_API UFINNetworkCircuit : public UObject {
	GENERATED_BODY()
	
	friend UFINNetworkConnector;

protected:
	TSet<FWeakObjectPtr> Nodes;

	void addNodeRecursive(TSet<UObject*>& added, UObject* add);

public:
	UFINNetworkCircuit();
	~UFINNetworkCircuit();

	/**
	 * Adds the given circuit to this circuit.
	 * Causes correct update signals for the components.
	 */
	UFINNetworkCircuit* operator+(UFINNetworkCircuit* circuit);
	
	/**
	 * Regenerates the node cache based on the given start component
	 */
	void recalculate(UObject* component);

	/**
	 * Returns if the given node is part of the circuit based on the circuit cache
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	bool HasNode(UObject* node);

	/**
	 * Trys to find the component with the given address/is in the circuit cache.
	 * @return	the found component, if it cant find it nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	UObject* FindComponent(FGuid addr);

	/**
	 * Trys to find components with the given nick in the circuit cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	TSet<UObject*> FindComponentsByNick(FString nick);

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
    static bool IsNodeConnected(UObject* start, UObject* node);

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
	static void DisconnectNodes(UObject* A, UObject* B);

	/**
	 * Updates the circuits of node A and B after they got connected
	 * by merging and recalculating the circuits.
	 * Should get called after the nodes got connected.
	 *
	 * @param[in]	A	the first component
	 * @param[in]	B	the second component
	 */
	UFUNCTION()
	static void ConnectNodes(UObject* A, UObject* B);

private:
	static bool IsNodeConnected_Internal(UObject* self, UObject* node, TSet<UObject*>& Searched);
};