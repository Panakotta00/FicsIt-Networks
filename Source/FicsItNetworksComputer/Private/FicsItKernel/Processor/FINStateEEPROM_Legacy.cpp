#include "FicsItKernel/Processor/FINStateEEPROM_Legacy.h"

#include "Net/UnrealNetwork.h"

AFINStateEEPROM_Legacy::AFINStateEEPROM_Legacy() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"RootComponent");
}

void AFINStateEEPROM_Legacy::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFINStateEEPROM_Legacy, Label);
}

bool AFINStateEEPROM_Legacy::ShouldSave_Implementation() const {
	return true;
}
