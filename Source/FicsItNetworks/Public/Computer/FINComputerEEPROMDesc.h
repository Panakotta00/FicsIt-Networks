#pragma once

#include "FicsItKernel/Processor/FINStateEEPROM.h"
#include "Utils/FINCopyableItem.h"
#include "FGInventoryComponent.h"
#include "Resources/FGEquipmentDescriptor.h"
#include "Tooltip/SMLItemDisplayInterface.h"
#include "FINComputerEEPROMDesc.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINComputerEEPROMDesc : public UFGItemDescriptor, public ISMLItemDisplayInterface, public IFINCopyableItemInterface {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFINStateEEPROM> EEPROMStateClass = nullptr;

	// Begin UFINCopyableItemInterface
	virtual bool CopyData_Implementation(UObject* WorldContext, const FInventoryItem& InFrom, const FInventoryItem& InTo, FInventoryItem& OutItem) override;
	// End UFINCopyableItemInterface

	// Begin ISMLItemDisplayInterface
	virtual FText GetOverridenItemName_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override;
	virtual FText GetOverridenItemDescription_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override { return FText(); }
	virtual UWidget* CreateDescriptionWidget_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override { return nullptr; }
	// End ISMLItemDisplayInterface
	
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	static AFINStateEEPROM* GetEEPROM(UFGInventoryComponent* Inv, int SlotIdx);
};
