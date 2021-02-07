#include "FINComputerEEPROMDesc.h"


#include "FGPlayerController.h"
#include "FINComputerRCO.h"

AFINStateEEPROM* UFINComputerEEPROMDesc::GetEEPROM(UFGInventoryComponent* Inv, int SlotIdx) {
	FInventoryStack stack;
	if (!IsValid(Inv) || !Inv->GetStackFromIndex(SlotIdx, stack) || !IsValid(stack.Item.ItemClass)) return nullptr;
	UFINComputerEEPROMDesc* desc = Cast<UFINComputerEEPROMDesc>(stack.Item.ItemClass->GetDefaultObject());
	if (!IsValid(desc)) return nullptr;
	UClass* clazz = desc->EEPROMStateClass;
	if (stack.Item.ItemState.IsValid()) {
		if (stack.Item.ItemState.Get()->GetClass()->IsChildOf(clazz)) return Cast<AFINStateEEPROM>(stack.Item.ItemState.Get());
	} else {
		UFINComputerRCO* RCO = Cast<UFINComputerRCO>(Cast<AFGPlayerController>(Inv->GetWorld()->GetFirstPlayerController())->GetRemoteCallObjectOfClass(UFINComputerRCO::StaticClass()));
		RCO->CreateEEPROMState(Inv, SlotIdx);
	}
	return nullptr;
}
