#include "FINComputerEEPROMDesc.h"

#include "FGPlayerController.h"
#include "FINComputerRCO.h"
#include "FINItemStateEEPROM.h"
#include "FicsItKernel/Processor/FINStateEEPROM_Legacy.h"

bool UFINComputerEEPROMDesc::CopyData_Implementation(UObject* WorldContext, const FInventoryItem& InFrom, const FInventoryItem& InTo, FInventoryItem& OutItem) {
	const FFINItemStateEEPROM* From = InFrom.GetItemState().GetValuePtr<FFINItemStateEEPROM>();
	OutItem = InTo;
	if (From == nullptr || !InTo.IsValid()) return false;

	UFINComputerEEPROMDesc* Descriptor = Cast<UFINComputerEEPROMDesc>(InTo.GetItemClass()->GetDefaultObject());
	if (!IsValid(Descriptor)) return false;
	OutItem.SetItemState(InFrom.GetItemState());
	return true;
}

FText UFINComputerEEPROMDesc::GetOverridenItemName_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) {
	FText Name = UFGItemDescriptor::GetItemName(InventoryStack.Item.GetItemClass());
	if (InventoryStack.Item.HasState()) {
		auto labelContainer = FFINStructInterfaces::Get().GetInterface<FFINLabelContainerInterface>(InventoryStack.Item.GetItemState());
		if (labelContainer) {
			FString Label = labelContainer->GetLabel();
			if (!Label.IsEmpty()) {
				return FText::FromString(FString::Printf(TEXT("%s - \"%s\""), *Name.ToString(), *Label));
			}
		}
	}
	return Name;
}

FFGDynamicStruct UFINComputerEEPROMDesc::GetEEPROM(UFGInventoryComponent* Inv, int SlotIdx) {
	FInventoryStack stack;
	if (!IsValid(Inv) || !Inv->GetStackFromIndex(SlotIdx, stack) || !IsValid(stack.Item.GetItemClass())) return FFGDynamicStruct();

	UFINComputerEEPROMDesc* desc = Cast<UFINComputerEEPROMDesc>(stack.Item.GetItemClass()->GetDefaultObject());
	if (!IsValid(desc)) return FFGDynamicStruct();

	if (stack.Item.HasState()) {
		return stack.Item.GetItemState();
	} else {
		AFGPlayerController* Controller = Cast<AFGPlayerController>(Inv->GetWorld()->GetFirstPlayerController());
		if (Controller) {
			UFINComputerRCO* RCO = Cast<UFINComputerRCO>(Controller->GetRemoteCallObjectOfClass(UFINComputerRCO::StaticClass()));
			RCO->CreateEEPROMState(Inv, SlotIdx);
			return stack.Item.GetItemState();
		}
	}
	return FFGDynamicStruct();
}

void UFINComputerEEPROMDesc::CreateEEPROMStateInItem(FInventoryItem& Item) {
	if (Item.GetItemClass()->IsChildOf<UFINComputerEEPROMDesc>() == false) return;
	if (Item.HasState() == false) {
		Item.SetItemState(Item.GetItemClass()->GetDefaultObject<UFINComputerEEPROMDesc>()->CreateEEPROMState());
	}
}

FFGDynamicStruct UFINComputerEEPROMDesc::CreateEEPROMState_Implementation() const {
	return FFGDynamicStruct();
}
