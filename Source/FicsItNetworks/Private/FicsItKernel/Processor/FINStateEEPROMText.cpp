#include "FicsItKernel/Processor/FINStateEEPROMText.h"

#include "Net/UnrealNetwork.h"

void AFINStateEEPROMText::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINStateEEPROMText, Code);
}

FString AFINStateEEPROMText::GetCode() const {
	return Code;
}

void AFINStateEEPROMText::SetCode(const FString& NewCode) {
	Code = NewCode;
	bShouldUpdate = true;
}

bool AFINStateEEPROMText::CopyDataTo(AFINStateEEPROM* InTo) {
	AFINStateEEPROMText* To = Cast<AFINStateEEPROMText>(InTo);
	if (!To) return false;
	To->SetCode(GetCode());
	return true;
}
