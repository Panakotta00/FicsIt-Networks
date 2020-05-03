#pragma once

#include "FGItemDescriptor.h"
#include "FINComputerDriveDesc.generated.h"

UCLASS()
class UFINComputerDriveDesc : public UFGItemDescriptor {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 StorageCapacity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		UTexture2D* DriveInventoryImage;

	UFINComputerDriveDesc();

	UFUNCTION(BlueprintCallable, Category = "FileSystem|Drive")
		static int32 GetStorageCapacity(TSubclassOf<UFINComputerDriveDesc> drive);

	UFUNCTION(BlueprintCallable, Category = "FileSystem|Drive")
		static UTexture2D* GetDriveInventoryImage(TSubclassOf<UFINComputerDriveDesc> drive);
};