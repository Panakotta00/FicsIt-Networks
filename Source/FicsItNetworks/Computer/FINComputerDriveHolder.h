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
	UFGInventoryComponent* DriveInventory = nullptr;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FFINDriveUpdate OnDriveUpdate;

	AFINComputerDriveHolder();
	~AFINComputerDriveHolder();

	AFINFileSystemState* GetDrive();

	UFUNCTION(BlueprintGetter)
		bool GetLocked() const;

	UFUNCTION(BlueprintSetter)
		bool SetLocked(bool NewLocked);

protected:
	UPROPERTY(SaveGame)
	AFINFileSystemState* prev = nullptr;

	UPROPERTY(SaveGame)
	bool bLocked = false;

	UFUNCTION(BlueprintNativeEvent, Category="Computer|Drive")
	void OnDriveInventoryUpdate(TSubclassOf<UFGItemDescriptor> drive, int32 count);
};