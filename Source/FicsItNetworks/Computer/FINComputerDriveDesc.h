#pragma once

#include "FGItemDescriptor.h"
#include "tooltip/ItemTooltipHandler.h"

#include "FINComputerDriveDesc.generated.h"

UCLASS()
class UFINComputerDriveDesc : public UFGItemDescriptor, public ISMLItemDisplayInterface {
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