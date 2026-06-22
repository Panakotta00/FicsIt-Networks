#include "FINComputerCaseWidget.h"

#include "FINComputerCase.h"

// FIN-1.2-PORT: Live-update bug on the very first EEPROM insert.
// The C++ delegate path (OnSlotUpdated -> OnEEPROMChanged -> Multicast ->
// OnEEPROMUpdate.Broadcast) is correct and fires on every insert. The bug was
// in the Blueprint-side binding, which missed the very first broadcast depending
// on Construct timing. Here we bind the delegate firmly in C++ and additionally
// push the current state on open -> the first event is never missed.

void UFINComputerCaseWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (Computer) {
		// Robust binding (AddUnique -> no double refresh if the BP also binds it).
		Computer->OnEEPROMUpdate.AddUniqueDynamic(this, &UFINComputerCaseWidget::OnEEPROMUpdate);
		// Push the current state to the UI immediately (correct display right on open).
		OnEEPROMUpdate(Computer->GetEEPROM());
	}
}

void UFINComputerCaseWidget::NativeDestruct() {
	if (Computer) {
		Computer->OnEEPROMUpdate.RemoveDynamic(this, &UFINComputerCaseWidget::OnEEPROMUpdate);
	}
	Super::NativeDestruct();
}
