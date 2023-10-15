#pragma once

#include "FicsItNetworks/Network/FINNetworkTrace.h"
#include "FIVSUtils.generated.h"

UCLASS()
class UFIVSUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:

	static FString NetworkTraceToString(const FFINNetworkTrace& Trace) {
		FString Str;
		const FFINNetworkTrace* CurrentTrace = &Trace;
		while (CurrentTrace) {
			Str += CurrentTrace->GetUnderlyingPtr()->GetPathName();
			Str += TEXT(";");
			if (CurrentTrace->GetStep()) Str += FFINNetworkTrace::inverseTraceStepRegistry[CurrentTrace->GetStep()];
			Str += TEXT(";");
			CurrentTrace = CurrentTrace->GetPrev().Get();
		}
		return Str;
	}

	static FFINNetworkTrace StringToNetworkTrace(FString Str) {
		FString Obj, Step;
		FFINNetworkTrace Trace;
		while (Str.Len() > 0) {
			if (!Str.Split(TEXT(";"), &Obj, &Str)) {
				Obj = Str;
				Step.Empty();
				Str.Empty();
			} else if (!Str.Split(TEXT(";"), &Step, &Str)) {
				Step = Str;
				Str.Empty();
			}
			UObject* ObjPtr = FSoftObjectPath(Obj).TryLoad();
			TSharedPtr<FFINTraceStep, ESPMode::ThreadSafe>* StepPtr = FFINNetworkTrace::traceStepRegistry.Find(Step);
			if (Step.IsEmpty() || !StepPtr) Trace = Trace / ObjPtr;
			else Trace = Trace.append(ObjPtr, *StepPtr);
		}
		return Trace.Reverse();
	}
};
