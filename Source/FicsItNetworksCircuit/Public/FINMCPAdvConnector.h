#pragma once

#include "CoreMinimal.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINMCPAdvConnector.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class FICSITNETWORKSCIRCUIT_API UFINMCPAdvConnector : public UFINAdvancedNetworkConnectionComponent {
	GENERATED_BODY()
public:
	UFINMCPAdvConnector();
};
