#pragma once

#include "FIRTrace.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FIVSUtils.generated.h"

UCLASS()
class UFIVSUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	static FString NetworkTraceToString(const FFIRTrace& Trace) {
		FString Str;
		const FFIRTrace* CurrentTrace = &Trace;
		while (CurrentTrace) {
			Str += CurrentTrace->GetUnderlyingPtr()->GetPathName();
			Str += TEXT(";");
			if (CurrentTrace->GetStep()) Str += FFIRTrace::inverseTraceStepRegistry[CurrentTrace->GetStep()];
			Str += TEXT(";");
			CurrentTrace = CurrentTrace->GetPrev().Get();
		}
		return Str;
	}

	static FFIRTrace StringToNetworkTrace(FString Str) {
		FString Obj, Step;
		FFIRTrace Trace;
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
			TSharedPtr<FFINTraceStep>* StepPtr = FFIRTrace::traceStepRegistry.Find(Step);
			if (Step.IsEmpty() || !StepPtr) Trace = Trace / ObjPtr;
			else Trace = Trace.Append(ObjPtr, *StepPtr);
		}
		return Trace.Reverse();
	}

	static TArray<FGuid> GuidsFromPins(TArrayView<UFIVSPin*> Pins) {
		TArray<FGuid> Guids;
		for (UFIVSPin* Pin : Pins) {
			Guids.Add(Pin->PinId);
		}
		return Guids;
	}
};
