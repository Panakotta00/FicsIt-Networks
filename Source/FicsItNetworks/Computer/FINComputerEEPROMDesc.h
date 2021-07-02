#pragma once

#include "FGInventoryComponent.h"
#include "FicsItNetworks/FicsItKernel/Processor/FINStateEEPROM.h"
#include "FicsItNetworks/Utils/FINCopyableItem.h"
#include "Resources/FGEquipmentDescriptor.h"
#include "FINComputerEEPROMDesc.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINComputerEEPROMDesc : public UFGItemDescriptor, public IFINCopyableItemInterface {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFINStateEEPROM> EEPROMStateClass = nullptr;

	// Begin UFINCopyableItemInterface
	virtual bool CopyData_Implementation(UObject* WorldContext, const FInventoryItem& InFrom, const FInventoryItem& InTo, FInventoryItem& OutItem) override;
	// End UFINCopyableItemInterface
	
	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	static AFINStateEEPROM* GetEEPROM(UFGInventoryComponent* Inv, int SlotIdx);
};
