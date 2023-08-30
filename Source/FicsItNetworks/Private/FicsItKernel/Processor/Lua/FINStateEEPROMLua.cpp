#include "FicsItKernel/Processor/Lua/FINStateEEPROMLua.h"

void AFINStateEEPROMLua::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINStateEEPROMLua, Code);
}

FString AFINStateEEPROMLua::GetCode() const {
	return Code;
}

void AFINStateEEPROMLua::SetCode(const FString& NewCode) {
	Code = NewCode;
	bShouldUpdate = true;
}

bool AFINStateEEPROMLua::CopyDataTo(AFINStateEEPROM* InTo) {
	AFINStateEEPROMLua* To = Cast<AFINStateEEPROMLua>(InTo);
	if (!To) return false;
	To->SetCode(GetCode());
	return true;
}
