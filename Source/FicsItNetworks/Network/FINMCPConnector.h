#pragma once
#include "FINNetworkConnectionComponent.h"
#include "FINMCPConnector.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class FICSITNETWORKS_API UFINMCPConnector : public UFINNetworkConnectionComponent {
	GENERATED_BODY()
public:
	UFINMCPConnector();
};
