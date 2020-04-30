#pragma once

#include "FINComputerModule.h"
#include "FGInventoryLibrary.h"
#include "FicsItKernel/FicsItFS/FINFileSystemState.h"
#include "FINComputerDriveHolder.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINDriveUpdate, bool, added, AFINFileSystemState*, drive);

UCLASS()
class AFINComputerDriveHolder : public AFINComputerModule {
	GENERATED_BODY()

public:
	UPROPERTY(SaveGame, BlueprintReadOnly)
	UFGInventoryComponent* DriveInventory;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FFINDriveUpdate OnDriveUpdate;

	AFINComputerDriveHolder();
	~AFINComputerDriveHolder();

	AFINFileSystemState* GetDrive();

private:
	UPROPERTY(SaveGame)
	AFINFileSystemState* prev;

	UFUNCTION()
	void OnDriveInventoryUpdate(TSubclassOf<UFGItemDescriptor> drive, int32 count);
};