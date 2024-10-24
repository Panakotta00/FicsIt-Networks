#pragma once

#include "CoreMinimal.h"
#include "FINComputerModule.h"
#include "FINItemStateFileSystem.h"
#include "FINComputerDriveHolder.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFINDriveHolderDriveUpdate, const FGuid&, Drive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFINDriveHolderLockedUpdateDelegate, bool, bOldLocked, const FGuid&, NewOrOldDrive);

UCLASS()
class FICSITNETWORKSCOMPUTER_API AFINComputerDriveHolder : public AFINComputerModule {
	GENERATED_BODY()

protected:
	UPROPERTY(SaveGame)
	FGuid PrevFSState;

	UPROPERTY(SaveGame, Replicated)
	bool bLocked = false;
	
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, SaveGame, Replicated)
	UFGInventoryComponent* DriveInventory = nullptr;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FFINDriveHolderDriveUpdate OnDriveUpdate;

	UPROPERTY(BlueprintAssignable)
	FFINDriveHolderLockedUpdateDelegate OnLockedUpdate;

	AFINComputerDriveHolder();
	~AFINComputerDriveHolder();

	// Begin AActor
	virtual void EndPlay(EEndPlayReason::Type reason) override;
	// End AActor

	FGuid GetDrive();

	UFUNCTION(BlueprintGetter)
	bool GetLocked() const;

	UFUNCTION(BlueprintSetter)
	bool SetLocked(bool NewLocked);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulti_OnDriveUpdate(const FGuid& Drive);

	UFUNCTION(NetMulticast, Unreliable)
	void NetMulti_OnLockedUpdate(bool bOldLocked, const FGuid& NewOrOldDrive);

protected:
	UFUNCTION(BlueprintNativeEvent, Category="Computer|Drive")
	void OnDriveInventoryUpdate(TSubclassOf<UFGItemDescriptor> drive, int32 count, UFGInventoryComponent* sourceInventory);
};