#include "FINLuaRCO.h"

#include "FGInventoryComponent.h"
#include "FGPlayerController.h"
#include "FINComputerEEPROMDesc.h"
#include "FINComputerRCO.h"
#include "FINItemStateEEPROMText.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

void UFINLuaRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFINLuaRCO, bDummy);
}
