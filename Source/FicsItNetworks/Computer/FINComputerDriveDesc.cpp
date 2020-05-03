#include "FINComputerDriveDesc.h"

UFINComputerDriveDesc::UFINComputerDriveDesc() {
	mStackSize = EStackSize::SS_ONE;
}

int32 UFINComputerDriveDesc::GetStorageCapacity(TSubclassOf<UFINComputerDriveDesc> drive) {
	return Cast<UFINComputerDriveDesc>(drive->GetDefaultObject())->StorageCapacity;
}

UTexture2D* UFINComputerDriveDesc::GetDriveInventoryImage(TSubclassOf<UFINComputerDriveDesc> drive) {
	return Cast<UFINComputerDriveDesc>(drive->GetDefaultObject())->DriveInventoryImage;
}
