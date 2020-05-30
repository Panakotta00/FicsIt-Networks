#include "FINStateEEPROM.h"

AFINStateEEPROM::AFINStateEEPROM() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"RootComponent");
}

bool AFINStateEEPROM::ShouldSave_Implementation() const {
	return true;
}
