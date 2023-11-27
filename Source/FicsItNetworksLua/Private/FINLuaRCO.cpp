#include "FINLuaRCO.h"

#include "FINStateEEPROMLua.h"
#include "Net/UnrealNetwork.h"

void UFINLuaRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFINLuaRCO, bDummy);
}

void UFINLuaRCO::SetLuaEEPROMCode_Implementation(AFINStateEEPROMLua* LuaEEPROMState, const FString& NewCode) {
	if (LuaEEPROMState) LuaEEPROMState->SetCode(NewCode);
}

bool UFINLuaRCO::SetLuaEEPROMCode_Validate(AFINStateEEPROMLua* LuaEEPROMState, const FString& NewCode) {
	return true;
}
