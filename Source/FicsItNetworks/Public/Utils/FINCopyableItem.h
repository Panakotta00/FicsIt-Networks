#pragma once

#include "FGInventoryComponent.h"
#include "SharedInventoryStatePtr.h"
#include "FINCopyableItem.generated.h"

UINTERFACE()
class UFINCopyableItemInterface : public UInterface {
	GENERATED_BODY()
};

class IFINCopyableItemInterface {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Computer")
	bool CopyData(UObject* WorldContext, const FInventoryItem& InFrom, const FInventoryItem& InTo, FInventoryItem& OutItem);
};
