#pragma once

#include "CoreMinimal.h"
#include "Interface.h"
#include "FINNetworkComponent.generated.h"

class UFINNetworkCircuit;

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
	 * Returns the array of components merged into this component.
	 * Allowing all objects returned to load signals, functions, etc. into the representation of this component.
	 * Merged components dont need to implement signal sender or other interface for allowing them to send signals etc.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
	TSet<UObject*> GetMerged() const;

	/**
	 * Allows the implementer to decided if the given network component by id
	 * has a more in-depth access to this component.
	 * This decided f.e. if a representation instance can get created by the
	 * given component for this component allowing to call functions and to listen.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Network|Component")
	bool AccessPermitted(FGuid ID) const;
	
	/**
	 * This function uses GetNick to check if the component has the given nick
	 */
	bool HasNickByNick(FString nick, FString has) const;
};