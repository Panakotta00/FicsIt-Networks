#pragma once
#include "FicsItNetworks/Network/FINNetworkCableHologram.h"
#include "FINMCPNetworkCableHologram.generated.h"

UCLASS()
class FICSITNETWORKS_API AFINMCPNetworkCableHologram : public AFINNetworkCableHologram {
	GENERATED_BODY()
public:
	AFINMCPNetworkCableHologram();

	virtual void GetRecipes(TSubclassOf<UFGRecipe>& OutRecipePole, TSubclassOf<UFGRecipe>& OutRecipePlug) override;
};
