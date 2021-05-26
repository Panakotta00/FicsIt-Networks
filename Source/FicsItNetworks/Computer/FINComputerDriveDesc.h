#pragma once

#include "Resources/FGItemDescriptor.h"
#include "Tooltip/SMLItemDisplayInterface.h"
#include "FINComputerDriveDesc.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINComputerDriveDesc : public UFGItemDescriptor, public ISMLItemDisplayInterface {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated)
	int32 StorageCapacity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UTexture2D* DriveInventoryImage = nullptr;

	UFINComputerDriveDesc();

	// Begin ISMLItemDisplayInterface
	virtual FText GetOverridenItemName_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override;
	virtual FText GetOverridenItemDescription_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override;
	virtual UWidget* CreateDescriptionWidget_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) override;
	// End ISMLItemDisplayInterface

	UFUNCTION(BlueprintCallable, Category = "FileSystem|Drive")
	static int32 GetStorageCapacity(TSubclassOf<UFINComputerDriveDesc> drive);

	UFUNCTION(BlueprintCallable, Category = "FileSystem|Drive")
	static UTexture2D* GetDriveInventoryImage(TSubclassOf<UFINComputerDriveDesc> drive);
};