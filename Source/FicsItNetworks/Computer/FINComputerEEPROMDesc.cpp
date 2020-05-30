#include "FINComputerEEPROMDesc.h"

AFINStateEEPROM* UFINComputerEEPROMDesc::GetEEPROM(UFGInventoryComponent* Inv, int SlotIdx) {
	FInventoryStack stack;
	if (!IsValid(Inv) || !Inv->GetStackFromIndex(SlotIdx, stack) || !IsValid(stack.Item.ItemClass)) return nullptr;
	UFINComputerEEPROMDesc* desc = Cast<UFINComputerEEPROMDesc>(stack.Item.ItemClass->GetDefaultObject());
	if (!IsValid(desc)) return nullptr;
	UClass* clazz = desc->EEPROMStateClass;
	if (stack.Item.ItemState.IsValid()) {
		if (stack.Item.ItemState.Get()->GetClass()->IsChildOf(clazz)) return Cast<AFINStateEEPROM>(stack.Item.ItemState.Get());
		else return nullptr;
	} else {
		FVector loc = FVector::ZeroVector;
		FRotator rot = FRotator::ZeroRotator;
		FActorSpawnParameters params;
		params.bNoFail = true;
		AFINStateEEPROM* eeprom = Inv->GetWorld()->SpawnActor<AFINStateEEPROM>(clazz, loc, rot, params);
		if (!IsValid(eeprom)) return nullptr;
		Inv->SetStateOnIndex(SlotIdx, FSharedInventoryStatePtr::MakeShared(eeprom));
		return eeprom;
	}
}
