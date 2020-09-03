#pragma once

#include "FINComputerModule.h"
#include "FGInventoryLibrary.h"
#include "FicsItKernel/FicsItFS/FINFileSystemState.h"
#include "FINComputerDriveHolder.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINDriveUpdate, bool, added, AFINFileSystemState*, drive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFINDriveHolderLockUpdated);

UCLASS()
class AFINComputerDriveHolder : public AFINComputerModule {
	GENERATED_BODY()

protected:
	UPROPERTY(SaveGame)
	AFINFileSystemState* prev = nullptr;

	UPROPERTY(SaveGame, ReplicatedUsing=OnLockedChanged)
	bool bLocked = false;
	
public:
	UPROPERTY(SaveGame, BlueprintReadOnly, Replicated)
	UFGInventoryComponent* DriveInventory = nullptr;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FFINDriveUpdate OnDriveUpdate;

	UPROPERTY(BlueprintAssignable)
	FFINDriveHolderLockUpdated OnLockUpdated;

	AFINComputerDriveHolder();
	~AFINComputerDriveHolder();

	AFINFileSystemState* GetDrive();

	UFUNCTION(BlueprintGetter)
	bool GetLocked() const;

	UFUNCTION(BlueprintSetter)
	bool SetLocked(bool NewLocked);

	UFUNCTION()
	void OnLockedChanged();

protected:
	UFUNCTION(BlueprintNativeEvent, Category="Computer|Drive")
	void OnDriveInventoryUpdate(TSubclassOf<UFGItemDescriptor> drive, int32 count);
};