#include "FINComputerDriveHolder.h"

#include "FINComputerDriveDesc.h"
#include "UnrealNetwork.h"

AFINComputerDriveHolder::AFINComputerDriveHolder() {
	DriveInventory = CreateDefaultSubobject<UFGInventoryComponent>("DriveInventory");
	DriveInventory->Resize();
	DriveInventory->OnItemAddedDelegate.AddDynamic(this, &AFINComputerDriveHolder::OnDriveInventoryUpdate);
	DriveInventory->OnItemRemovedDelegate.AddDynamic(this, &AFINComputerDriveHolder::OnDriveInventoryUpdate);
	DriveInventory->SetIsReplicated(true);
}

AFINComputerDriveHolder::~AFINComputerDriveHolder() {}

void AFINComputerDriveHolder::EndPlay(EEndPlayReason::Type reason) {
	SetLocked(false);
	Super::EndPlay(reason);
}

void AFINComputerDriveHolder::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerDriveHolder, DriveInventory);
	DOREPLIFETIME(AFINComputerDriveHolder, bLocked);
}

AFINFileSystemState* AFINComputerDriveHolder::GetDrive() {
	FInventoryStack stack;
	if (DriveInventory->GetStackFromIndex(0, stack)) {
		TSubclassOf<UFINComputerDriveDesc> driveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.ItemClass);
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

bool AFINComputerDriveHolder::GetLocked() const {
	return bLocked;
}

bool AFINComputerDriveHolder::SetLocked(bool NewLocked) {
	if (!HasAuthority()) return false;
	AFINFileSystemState* newState = GetDrive();
	if (bLocked == NewLocked || (!IsValid(newState) && NewLocked)) return false;

	bLocked = NewLocked;
	
	NetMulti_OnLockedUpdate(!bLocked, IsValid(newState) ? newState : PrevFSState);
	PrevFSState = newState;

	return true;
}

void AFINComputerDriveHolder::NetMulti_OnDriveUpdate_Implementation(AFINFileSystemState* Drive) {
	OnDriveUpdate.Broadcast(Drive);
}

void AFINComputerDriveHolder::NetMulti_OnLockedUpdate_Implementation(bool bOldLocked, AFINFileSystemState* NewOrOldDrive) {
	OnLockedUpdate.Broadcast(bOldLocked, NewOrOldDrive);
}

void AFINComputerDriveHolder::OnDriveInventoryUpdate_Implementation(TSubclassOf<UFGItemDescriptor> drive, int32 count) {
	if (HasAuthority()) {
		NetMulti_OnDriveUpdate(GetDrive());
		if (!IsValid(drive)) SetLocked(false);
	}
}
