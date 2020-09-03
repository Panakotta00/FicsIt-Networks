#include "FINStateEEPROM.h"

AFINStateEEPROM::AFINStateEEPROM() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"RootComponent");

	bReplicates = true;
	bAlwaysRelevant = true;
}

bool AFINStateEEPROM::ShouldSave_Implementation() const {
	return true;
}

void AFINStateEEPROM::OnCodeUpdate_Implementation() {
	UpdateDelegate.Broadcast();
}
