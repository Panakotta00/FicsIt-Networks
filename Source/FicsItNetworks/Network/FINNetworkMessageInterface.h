#pragma once

#include "CoreMinimal.h"
#include "Interface.h"

#include "FINNetworkMessageInterface.generated.h"

UINTERFACE(Blueprintable)
class UFINNetworkMessageInterface : public UInterface {
	GENERATED_BODY()
};

class IFINNetworkMessageInterface {
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintImplementableEvent, Category="Network|Messages")
	bool IsPortOpen(int Port);
};