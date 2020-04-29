#include "FINComputerDriveHolder.h"

#include "FINComputerDriveDesc.h"

AFINComputerDriveHolder::AFINComputerDriveHolder() {
	DriveInventory = CreateDefaultSubobject<UFGInventoryComponent>("DriveInventory");
	DriveInventory->Resize();
	DriveInventory->OnItemAddedDelegate.AddDynamic(this, &AFINComputerDriveHolder::OnDriveInventoryUpdate);
	DriveInventory->OnItemRemovedDelegate.AddDynamic(this, &AFINComputerDriveHolder::OnDriveInventoryUpdate);
}

AFINComputerDriveHolder::~AFINComputerDriveHolder() {}

AFINFileSystemState* AFINComputerDriveHolder::GetDrive() {
	FInventoryStack stack;
	if (DriveInventory->GetStackFromIndex(0, stack)) {
		TSubclassOf<UFINComputerDriveDesc> driveDesc = stack.Item.ItemClass;
		AFINFileSystemState* state = Cast<AFINFileSystemState>(stack.Item.ItemState.Get());
		if (IsValid(driveDesc)) {
			if (!IsValid(state)) {
				state = AFINFileSystemState::CreateState(this, UFINComputerDriveDesc::GetStorageCapacity(driveDesc), DriveInventory, 0);
			}
			return state;
		}
	}
	return nullptr;
}

void AFINComputerDriveHolder::OnDriveInventoryUpdate(TSubclassOf<UFGItemDescriptor> drive, int32 count) {
	OnDriveUpdate.Broadcast(GetDrive());
}
