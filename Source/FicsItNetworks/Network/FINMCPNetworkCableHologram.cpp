#include "FINMCPNetworkCableHologram.h"
#include "FINMCPConnector.h"

AFINMCPNetworkCableHologram::AFINMCPNetworkCableHologram() {
	bEnablePole = false;
}

void AFINMCPNetworkCableHologram::GetRecipes(TSubclassOf<UFGRecipe>& OutRecipePole, TSubclassOf<UFGRecipe>& OutRecipePlug) {
	Super::GetRecipes(OutRecipePole, OutRecipePlug);
	OutRecipePlug = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/Components/SmallNetworkWallPlug/Recipe_SmallNetworkWallPlug.Recipe_SmallNetworkWallPlug_C"));
}
