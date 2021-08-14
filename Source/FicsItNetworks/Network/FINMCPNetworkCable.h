#pragma once
#include "FINNetworkCable.h"
#include "FINMCPNetworkCable.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINMCPNetworkCable : public AFINNetworkCable {
	GENERATED_BODY()
public:

	AFINMCPNetworkCable();

	virtual void OnConstruction(const FTransform& Transform) override;

};
