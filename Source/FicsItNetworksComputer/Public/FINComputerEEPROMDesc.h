#pragma once

#include "CoreMinimal.h"
#include "FINCopyableItem.h"
#include "Resources/FGItemDescriptor.h"
#include "Tooltip/SMLItemDisplayInterface.h"
#include "FINComputerEEPROMDesc.generated.h"

/**
 * A EEPROM can hold some data that is used by computers as configuration like code.
 *
 * The workflow for different operations is as follows:
 * - Get The EEPROM:
 *   1. Get the Inventory or Inventory Item
 *   2. From it, get the Item State (no construction)
 *   3. try to convert it to the struct type you need
 * - Set The EEPROM:
 *   1. Get the Inventory of Inventory Item
 *   2. From it, get the Item State or let the default construction occur
 *   3. Try to convert it to the struct type you need, if this was not successful you know either the item is no EEPROM, or simply not the EEPROM type you need.
 *   4. If successful, change the value to however you need.
 *   5. Save the changed EEPROM state to the item of the original inventory.
 */
UCLASS()
class FICSITNETWORKSCOMPUTER_API UFINComputerEEPROMDesc : public UFGItemDescriptor, public ISMLItemDisplayInterface, public IFINCopyableItemInterface {
	GENERATED_BODY()
public:
	// Begin UFINCopyableItemInterface
	virtual bool CopyData_Implementation(UObject* WorldContext, const FInventoryItem& InFrom, const FInventoryItem& InTo, FInventoryItem& OutItem) override;
	// End UFINCopyableItemInterface

	// Begin ISMLItemDisplayInterface
	virtual FText GetOverridenItemName_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override;
	virtual FText GetOverridenItemDescription_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override { return FText(); }
	virtual UWidget* CreateDescriptionWidget_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override { return nullptr; }
	// End ISMLItemDisplayInterface

	/**
	 * Tries to get a valid EEPROM Item State from stack at the given index in the given inventory.
	 * If item is not a EEPROM Type, returns an invalid struct.
	 * If no state is set, creates a state on the host. Afterwards returns an invalid struct on clients (due to latency) and the valid struct on host.
	 * If a state is set, returns that state.
	 *
	 * An invalid struct is the same as no data set, and so, if the state first has to be created, the default state will also contain no data, which is the same as an invalid struct.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	static FFGDynamicStruct GetEEPROM(UFGInventoryComponent* Inv, int SlotIdx);

	/**
	 * Creates an EEPROM State for the given item if it does not already exist.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	static void CreateEEPROMStateInItem(FInventoryItem& Item);

	/**
	 * Host Only to create the EEPROM State Polymorphicly
	 */
	UFUNCTION(BlueprintNativeEvent)
	FFGDynamicStruct CreateEEPROMState() const;
};
