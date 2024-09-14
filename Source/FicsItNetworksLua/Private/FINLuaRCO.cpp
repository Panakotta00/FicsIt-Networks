#include "FINLuaRCO.h"

#include "FicsItKernel/Processor/FINStateEEPROMText.h"
#include "Net/UnrealNetwork.h"

void UFINLuaRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFINLuaRCO, bDummy);
}

void UFINLuaRCO::SetLuaEEPROMCode_Implementation(AFINStateEEPROMText* LuaEEPROMState, const FString& NewCode) {
	if (LuaEEPROMState) LuaEEPROMState->SetCode(NewCode);
}

bool UFINLuaRCO::SetLuaEEPROMCode_Validate(AFINStateEEPROMText* LuaEEPROMState, const FString& NewCode) {
	return true;
}
