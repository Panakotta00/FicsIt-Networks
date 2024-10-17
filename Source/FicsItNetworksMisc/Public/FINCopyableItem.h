#pragma once

#include "CoreMinimal.h"
#include "FINCopyableItem.generated.h"

UINTERFACE()
class FICSITNETWORKSMISC_API UFINCopyableItemInterface : public UInterface {
	GENERATED_BODY()
};

class FICSITNETWORKSMISC_API IFINCopyableItemInterface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Computer")
	bool CopyData(UObject* WorldContext, const FInventoryItem& InFrom, const FInventoryItem& InTo, FInventoryItem& OutItem);
};
