#include "FINFuture.h"

FCriticalSection FFINFutureReflection::Mutex;

void FFINFutureReflection::Execute() {
	FScopeLock Lock(&Mutex);

	// BUG #16: This Execute() runs on the GAME thread (runtime-0 / RT_Synchronized). If
	// Function->Execute or a property setter/getter throws an FFIRException here, it would otherwise
	// fly through uncaught -> [Fatal] -> computer kernel halted (luaFIN_callReflectionFunctionDirectly's
	// try/catch only covers the RT_Async/RT_Parallel path, NOT the future). So catch it here and report
	// it back via bError/ErrorMessage; lua_futureStructContinue turns that into a catchable Lua error.
	// bDone is ALWAYS set, otherwise luaFIN_await would hang forever.
	try {
		if (!Context.IsValid()) {
			throw FFIRException(TEXT("Execution context of future is invalid."));
		}

		if (Function) {
			Output = Function->Execute(Context, Input);
		} else if (Property) {
			if (Input.Num() > 0) {
				Property->SetValue(Context, Input[0]);
			} else {
				Output.Add(Property->GetValue(Context));
			}
		} else {
			UE_LOG(LogFicsItNetworksMisc, Error, TEXT("Future unable to get executed due to invalid function/property pointer!"));
		}
	} catch (const FFIRException& Ex) {
		bError = true;
		ErrorMessage = Ex.GetMessage();
	} catch (...) {
		bError = true;
		ErrorMessage = TEXT("Unhandled C++ exception in reflection future execution");
	}
	bDone = true;
}
