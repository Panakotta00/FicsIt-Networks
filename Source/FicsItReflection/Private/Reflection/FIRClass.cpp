#include "Reflection/FIRClass.h"

UFIRSignal* UFIRClass::FindFIRSignal(const FString& Name) {
	for (UFIRSignal* Signal : GetSignals()) {
		if (Signal->GetInternalName() == Name) return Signal;
	}
	return nullptr;
}
