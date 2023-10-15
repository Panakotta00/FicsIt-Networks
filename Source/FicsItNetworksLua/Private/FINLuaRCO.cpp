#include "FINLuaRCO.h"

void UFINLuaRCO::SetLuaEEPROMCode_Implementation(AFINStateEEPROMLua* LuaEEPROMState, const FString& NewCode) {
	LuaEEPROMState->SetCode(NewCode);
}

bool UFINLuaRCO::SetLuaEEPROMCode_Validate(AFINStateEEPROMLua* LuaEEPROMState, const FString& NewCode) {
	return true;
}
