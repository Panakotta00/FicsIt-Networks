#pragma once

#include "Interface.h"
#include "FINNetworkCustomType.generated.h"

UINTERFACE()
class UFINNetworkCustomType : public UInterface {
	GENERATED_BODY()
};

class IFINNetworkCustomType {
	GENERATED_BODY()
public:
	/**
	 * Returns the typename of this custom network type
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Network|Types")
	FString GetCustomTypeName() const;
};
