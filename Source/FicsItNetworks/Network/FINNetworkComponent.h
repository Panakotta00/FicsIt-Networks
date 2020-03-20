#pragma once

#include "CoreMinimal.h"
#include "Interface.h"
#include "FINNetworkCircuit.h"
#include "FINNetworkComponent.generated.h"

/**
 * A Network Component implements functions allowing to interact with the computer network.
 * It is also nessesery to identify the components in the network or to build the node network.
 */
UINTERFACE(Blueprintable)
class FICSITNETWORKS_API UFINNetworkComponent : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKS_API IFINNetworkComponent {
	GENERATED_IINTERFACE_BODY()

public:
	/**
	* Returns the GUID used to refer this node.
	* This ID is used to uniquely identfy the component in the network.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		FGuid GetID() const;

	/**
	 * Returns the nick of this component.
	 * The nick is used to group components and to give components an alias.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		FString GetNick() const;

	/**
	 * Sets the nick of this component.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		void SetNick(const FString& nick);

	/**
	 * Checks if this node has the given nick.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		bool HasNick(const FString& nick);

	/**
	 * Returns the array of components merged into this coponent.
	 * Allowing all objects returned to load signals, functions, etc. into the representation of this component.
	 * Merged components dont need to implement signal sender or other interface for allowing them to send signals etc.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		TSet<UObject*> GetMerged() const;

	/**
	 * Returns a array of connected components.
	 * These components are used to refer to "neighbours" in the network and so, actualy create a network of components.
	 * Contains only the direct neighbours of the node.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		TSet<UObject*> GetConnected() const;

	/**
	 * Trys to find the component with the given ID in the network.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		FFINNetworkTrace FindComponent(FGuid guid) const;

	/**
	 * Returns the connected network circuit of this node.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		UFINNetworkCircuit* GetCircuit() const;

	/**
	 * Sets the connected network circuit of this node.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		void SetCircuit(UFINNetworkCircuit* circuit);

	/**
	 * This functions gets executed when a change in the computer network circuit occured.
	 * Like adding or removing a new component.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
		void NotifyNetworkUpdate(int32 type, const TSet<FFINNetworkTrace>& nodes);

	/**
	 * This functions uses GetCircuit to search in the circuit for the component witht he given guid
	 */
	FFINNetworkTrace FindComponentByCircuit(FGuid guid) const;

	/**
	 * This function uses GetNick to check if the component has the given nick
	 */
	bool HasNickByNick(FString nick) const;
};