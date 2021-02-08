#include "FINStateEEPROM.h"

AFINStateEEPROM::AFINStateEEPROM() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"RootComponent");

	bReplicates = true;
	bAlwaysRelevant = true;

	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(true);
}

void AFINStateEEPROM::Tick(float DeltaSeconds) {
	Super::Tick(DeltaSeconds);

	if (bShouldUpdate) {
		bShouldUpdate = false;
		OnCodeUpdate();
	}
}

bool AFINStateEEPROM::ShouldSave_Implementation() const {
	return true;
}

void AFINStateEEPROM::OnCodeUpdate_Implementation() {
	UpdateDelegate.Broadcast();
}
