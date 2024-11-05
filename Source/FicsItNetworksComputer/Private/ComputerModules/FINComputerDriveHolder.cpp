#include "ComputerModules/FINComputerDriveHolder.h"

#include "FGPlayerController.h"
#include "FINFileSystemSubsystem.h"
#include "ComputerModules/FINComputerDriveDesc.h"
#include "FicsItKernel/FicsItFS/FINItemStateFileSystem.h"
#include "Net/UnrealNetwork.h"

AFINComputerDriveHolder::AFINComputerDriveHolder() {
	DriveInventory = CreateDefaultSubobject<UFGInventoryComponent>("DriveInventory");
	DriveInventory->SetDefaultSize(1);
	DriveInventory->Resize(1);

	bReplicates = true;
	NetDormancy = DORM_Awake;
}

void AFINComputerDriveHolder::BeginPlay() {
	Super::BeginPlay();

	DriveInventory->OnSlotUpdatedDelegate.AddDynamic(this, &AFINComputerDriveHolder::OnDriveSlotUpdate);

	DriveInventory->mItemFilter.BindLambda([](TSubclassOf<UObject> Item, int32 Index) {
		//return Item->IsChildOf(UFINComputerDriveDesc::StaticClass());
		return true;
	});
	if (HasAuthority()) {
		DriveInventory->SetReplicationRelevancyOwner(this);
		DriveInventory->Resize(1);
	}
}

void AFINComputerDriveHolder::EndPlay(EEndPlayReason::Type reason) {
	SetLocked(false);
	Super::EndPlay(reason);
}

void AFINComputerDriveHolder::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerDriveHolder, bLocked);
}

FGuid AFINComputerDriveHolder::GetDrive() {
	FInventoryStack stack;
	if (DriveInventory->GetStackFromIndex(0, stack)) {
		TSubclassOf<UFINComputerDriveDesc> driveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.GetItemClass());
		const FFINItemStateFileSystem* state = stack.Item.GetItemState().GetValuePtr<FFINItemStateFileSystem>();
		if (IsValid(driveDesc)) {
			if (state) {
				return state->ID;
			} else {
				return AFINFileSystemSubsystem::GetFileSystemSubsystem(this)->CreateState(UFINComputerDriveDesc::GetStorageCapacity(driveDesc), DriveInventory, 0);
			}
		}
	}
	static FGuid invalidGuid;
	return invalidGuid;
}

bool AFINComputerDriveHolder::GetLocked() const {
	return bLocked;
}

bool AFINComputerDriveHolder::SetLocked(bool NewLocked) {
	if (!HasAuthority()) return false;
	FGuid newState = GetDrive();
	if (bLocked == NewLocked || (!newState.IsValid() && NewLocked)) return false;

	bLocked = NewLocked;
	
	NetMulti_OnLockedUpdate(!bLocked, newState.IsValid() ? newState : PrevFSState);
	PrevFSState = newState;

	return true;
}

void AFINComputerDriveHolder::OnDriveSlotUpdate(int index) {
	FGuid drive = GetDrive();
	if (HasAuthority()) {
		NetMulti_OnDriveUpdate(drive);
		SetLocked(drive.IsValid());
	} else {
		OnDriveUpdate.Broadcast(drive);
	}
}

void AFINComputerDriveHolder::NetMulti_OnDriveUpdate_Implementation(const FGuid& Drive) {
	OnDriveUpdate.Broadcast(Drive);
}

void AFINComputerDriveHolder::NetMulti_OnLockedUpdate_Implementation(bool bOldLocked, const FGuid& NewOrOldDrive) {
	OnLockedUpdate.Broadcast(bOldLocked, NewOrOldDrive);
}
