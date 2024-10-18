#pragma once

#include "CoreMinimal.h"
#include "FINNetworkConnectionComponent.h"
#include "FINMCPConnector.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class FICSITNETWORKSCIRCUIT_API UFINMCPConnector : public UFINNetworkConnectionComponent {
	GENERATED_BODY()
public:
	UFINMCPConnector();
};
