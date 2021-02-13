#pragma once

#include "FGEquipmentDescriptor.h"
#include "FGInventoryComponent.h"
#include "FicsItKernel/Processor/FINStateEEPROM.h"

#include "FINComputerEEPROMDesc.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINComputerEEPROMDesc : public UFGEquipmentDescriptor {
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AFINStateEEPROM> EEPROMStateClass = nullptr;

	UFUNCTION(BlueprintCallable, Category="Network|Computer")
	static AFINStateEEPROM* GetEEPROM(UFGInventoryComponent* Inv, int SlotIdx);
};
