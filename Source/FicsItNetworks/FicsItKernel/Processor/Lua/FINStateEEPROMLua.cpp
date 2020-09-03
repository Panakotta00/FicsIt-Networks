#include "FINStateEEPROMLua.h"

#include "UnrealNetwork.h"

void AFINStateEEPROMLua::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINStateEEPROMLua, Code);
}

FString AFINStateEEPROMLua::GetCode() const {
	return Code;
}

void AFINStateEEPROMLua::SetCode(const FString& Code) {
	this->Code = Code;
	OnCodeUpdate();
}
