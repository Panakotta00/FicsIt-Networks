#include "FINFuture.h"

FCriticalSection FFINFutureReflection::Mutex;

void FFINFutureReflection::Execute() {
	FScopeLock Lock(&Mutex);

	if (!Context.IsValid()) {
		throw FFIRException(TEXT("Execution context of future is invalid."));
	}

	if (Function) {
		Output = Function->Execute(Context, Input);
		bDone = true;
	} else if (Property) {
		if (Input.Num() > 0) {
			Property->SetValue(Context, Input[0]);
		} else {
			Output.Add(Property->GetValue(Context));
		}
		bDone = true;
	} else {
		UE_LOG(LogFicsItNetworksMisc, Error, TEXT("Future unable to get executed due to invalid function/property pointer!"));
	}
}
