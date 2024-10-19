#include "Reflection/FIRSignal.h"

#include "FicsItReflection.h"

void UFIRSignal::Trigger(UObject* Context, const TArray<FFIRAnyValue>& Data) {
	FFicsItReflectionModule::Get().OnSignalTriggered.Broadcast(Context, this, Data);
}
