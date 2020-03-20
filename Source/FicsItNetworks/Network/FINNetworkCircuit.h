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
	UPROPERTY()
	TSet<TWeakObjectPtr<UObject>> Nodes;

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
	 * Returns if the given node is part of the circuit
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	bool HasNode(UObject* node);

	/**
	 * Trys to find the component with the given address/is in the circuit cache.
	 * @return	the found component, if it cant find it nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	FFINNetworkTrace FindComponent(FGuid addr);

	/**
	 * Trys to find components with the given nick in the circuit cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	TSet<FFINNetworkTrace> FindComponentsByNick(FString nick);

	/**
	 * Returns all components in the circuit cache.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Circuit")
	void GetComponents(UPARAM(ref) TSet<FFINNetworkTrace>& out_components);
};