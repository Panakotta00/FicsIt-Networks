#pragma once

#include "FGItemDescriptor.h"
#include "FINComputerDriveDesc.generated.h"

UCLASS()
class UFINComputerDriveDesc : public UFGItemDescriptor {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		int32 StorageCapacity;

	UFUNCTION(BlueprintCallable, Category = "FileSystem|Drive")
		static int32 GetStorageCapacity(TSubclassOf<UFINComputerDriveDesc> drive);
};