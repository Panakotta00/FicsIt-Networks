#include "Computer/FINComputerEEPROMDesc.h"
#include "FGPlayerController.h"
#include "Computer/FINComputerRCO.h"

bool UFINComputerEEPROMDesc::CopyData_Implementation(UObject* WorldContext, const FInventoryItem& InFrom, const FInventoryItem& InTo, FInventoryItem& OutItem) {
	AFINStateEEPROM* From = Cast<AFINStateEEPROM>(InFrom.ItemState.Get());
	AFINStateEEPROM* To = Cast<AFINStateEEPROM>(InTo.ItemState.Get());
	OutItem = InTo;
	if (!To) {
		if (!InTo.IsValid()) return false; 
		UFINComputerEEPROMDesc* Descriptor = Cast<UFINComputerEEPROMDesc>(InTo.GetItemClass()->GetDefaultObject());
		if (!IsValid(Descriptor)) return false;
		UClass* StateClass = Descriptor->EEPROMStateClass;
		FVector loc = FVector::ZeroVector;
		FRotator rot = FRotator::ZeroRotator;
		FActorSpawnParameters params;
		params.bNoFail = true;
		To = WorldContext->GetWorld()->SpawnActor<AFINStateEEPROM>(StateClass, loc, rot, params);
		if (To) OutItem.ItemState = FSharedInventoryStatePtr::MakeShared(To);
	}
	if (!From || !To) return false;
	return From->CopyDataTo(To);
}

AFINStateEEPROM* UFINComputerEEPROMDesc::GetEEPROM(UFGInventoryComponent* Inv, int SlotIdx) {
	FInventoryStack stack;
	if (!IsValid(Inv) || !Inv->GetStackFromIndex(SlotIdx, stack) || !IsValid(stack.Item.GetItemClass())) return nullptr;
	UFINComputerEEPROMDesc* desc = Cast<UFINComputerEEPROMDesc>(stack.Item.GetItemClass()->GetDefaultObject());
	if (!IsValid(desc)) return nullptr;
	UClass* clazz = desc->EEPROMStateClass;
	if (stack.Item.ItemState.IsValid()) {
		if (stack.Item.ItemState.Get()->GetClass()->IsChildOf(clazz)) return Cast<AFINStateEEPROM>(stack.Item.ItemState.Get());
	} else {
		AFGPlayerController* Controller = Cast<AFGPlayerController>(Inv->GetWorld()->GetFirstPlayerController());
		if (Controller) {
			UFINComputerRCO* RCO = Cast<UFINComputerRCO>(Controller->GetRemoteCallObjectOfClass(UFINComputerRCO::StaticClass()));
			RCO->CreateEEPROMState(Inv, SlotIdx);
		}
	}
	return nullptr;
}
