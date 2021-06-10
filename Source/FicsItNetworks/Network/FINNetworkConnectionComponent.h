#pragma once

#include "FGSaveInterface.h"
#include "FINNetworkCircuitNode.h"
#include "Components/SceneComponent.h"
#include "FINNetworkConnectionComponent.generated.h"

class AFINNetworkCable;

/**
 * This actor component is a network circuit node which allows you
 * to have a network cable connection point.
 * This component is only used for the network circuit and doesn't implement
 * any network component logic. Use UFINNetworkConnector instead if you want so.
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class FICSITNETWORKS_API UFINNetworkConnectionComponent : public USceneComponent, public IFINNetworkCircuitNode, public IFGSaveInterface {
	GENERATED_BODY()
public:
	/**
	 * The maximum amount of cables you can connect to this connector.
	 */
	UPROPERTY(EditDefaultsOnly)
	int MaxCables = -1;

	/**
	 * The "hidden" connections to other network connectors.
	 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	TSet<TSoftObjectPtr<UObject>> ConnectedNodes;
	
	/**
	 * The connected cables to this network connector.
	 */
	UPROPERTY()
	TSet<AFINNetworkCable*> ConnectedCables;

	/**
	 * The computer network circuit this connector is connected to.
	 */
	UPROPERTY(Replicated)
	AFINNetworkCircuit* Circuit = nullptr;

	// Begin UObject
	virtual void InitializeComponent() override;
	// End UObject

	// Begin IFINNetworkCircuitNode
	virtual TSet<UObject*> GetConnected_Implementation() const override;
	virtual AFINNetworkCircuit* GetCircuit_Implementation() const override;
	virtual void SetCircuit_Implementation(AFINNetworkCircuit* Circuit) override;
	virtual void NotifyNetworkUpdate_Implementation(int Type, const TSet<UObject*>& Nodes) override;
	// End IFINNetworkCircuitNode

	/**
	 * adds the given node as connection to this connector.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Connector")
    void AddConnectedNode(TScriptInterface<IFINNetworkCircuitNode> Node);

	/**
	 * removes the given node as connection from this connector.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Connector")
    void RemoveConnectedNode(TScriptInterface<IFINNetworkCircuitNode> Node);

	/**
	 * returns a list of all connected cables
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Connector")
	TSet<AFINNetworkCable*> GetConnectedCables();

	/**
	 * adds the given network cable to this connector.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Connector")
    bool AddConnectedCable(AFINNetworkCable* Cable);

	/**
	 * removes the given network cable from this connector.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Connector")
    void RemoveConnectedCable(AFINNetworkCable* Cable);

	/**
	 * Checks if the given network connection component is connected to this component.
	 */
	UFUNCTION(BlueprintCallable, Category = "Network|Connector")
    bool IsConnected(const TScriptInterface<IFINNetworkCircuitNode>& Node) const;
};
