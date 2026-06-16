#include "FINComputerCaseWidget.h"

#include "FINComputerCase.h"

// FIN-1.2-PORT: Live-Update-Bug beim allerersten EEPROM-Einsetzen.
// Der C++-Delegate-Pfad (OnSlotUpdated -> OnEEPROMChanged -> Multicast ->
// OnEEPROMUpdate.Broadcast) ist korrekt und feuert bei jedem Insert. Der Bug lag
// in der Blueprint-seitigen Bindung, die das allererste Broadcast je nach
// Construct-Timing verpasste. Hier binden wir den Delegate fest in C++ und pushen
// zusaetzlich den aktuellen Stand beim Oeffnen -> erstes Event wird nie verpasst.

void UFINComputerCaseWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (Computer) {
		// Robuste Bindung (AddUnique -> kein Doppel-Refresh, falls das BP es auch bindet).
		Computer->OnEEPROMUpdate.AddUniqueDynamic(this, &UFINComputerCaseWidget::OnEEPROMUpdate);
		// Aktuellen Stand sofort an die UI pushen (korrekte Anzeige direkt beim Oeffnen).
		OnEEPROMUpdate(Computer->GetEEPROM());
	}
}

void UFINComputerCaseWidget::NativeDestruct() {
	if (Computer) {
		Computer->OnEEPROMUpdate.RemoveDynamic(this, &UFINComputerCaseWidget::OnEEPROMUpdate);
	}
	Super::NativeDestruct();
}
