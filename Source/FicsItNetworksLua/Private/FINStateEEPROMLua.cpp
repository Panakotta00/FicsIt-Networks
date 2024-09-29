#include "FINStateEEPROMLua.h"

#include "Net/UnrealNetwork.h"

void AFINStateEEPROMLua::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINStateEEPROMLua, Code);
}

FString AFINStateEEPROMLua::GetCode() const {
	return Code;
}

void AFINStateEEPROMLua::SetCode(const FString& NewCode) {
	Code = NewCode;
}

bool AFINStateEEPROMLua::CopyDataTo(AFINStateEEPROM_Legacy* InTo) {
	AFINStateEEPROMLua* To = Cast<AFINStateEEPROMLua>(InTo);
	if (!To) return false;
	To->SetCode(GetCode());
	return true;
}
