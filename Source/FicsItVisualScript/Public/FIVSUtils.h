#pragma once

#include "FIRTrace.h"
#include "FIVSPin.h"
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
			if (Trace.IsValidPtr()) {
				if (Step.IsEmpty() || !StepPtr) Trace = Trace / ObjPtr;
				else Trace = Trace.Append(ObjPtr, *StepPtr);
			} else {
				Trace = FFIRTrace(ObjPtr);
			}
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

FICSITVISUALSCRIPT_API FString FIRObjectToString(UObject* InObj);
FICSITVISUALSCRIPT_API FString FIRClassToString(UClass* InClass);

FICSITVISUALSCRIPT_API FORCEINLINE FFIRAnyValue FIRCastValue(const FFIRAnyValue& Value, EFIRValueType ToType) {
	if (Value.GetType() == FIR_ANY) return FIRCastValue(Value.GetAny(), ToType);

	switch (ToType) {
	case FIR_BOOL:
		switch (Value.GetType()) {
		case FIR_NIL:
			return false;
		case FIR_BOOL:
			return Value;
		case FIR_INT:
			return Value.GetInt() != 0;
		case FIR_FLOAT:
			return Value.GetFloat() != 0.0;
		case FIR_STR:
			return Value.GetString() == TEXT("true");
		case FIR_OBJ:
			return Value.GetObj().IsValid();
		case FIR_CLASS:
			return Value.GetClass() != nullptr;
		case FIR_TRACE:
			return Value.GetTrace().IsValid();
		case FIR_ARRAY:
			return Value.GetArray().Num() > 0;
		default: ;
		}
		break;
	case FIR_INT:
		switch (Value.GetType()) {
		case FIR_NIL:
			return 1ll;
		case FIR_BOOL:
			return Value.GetBool() ? 1ll : 0ll;
		case FIR_INT:
			return Value;
		case FIR_FLOAT:
			return (FIRInt)Value.GetFloat();
		case FIR_STR: {
			FIRInt ValInt = 0;
			FDefaultValueHelper::ParseInt64(Value.GetString(), ValInt);
			return ValInt;
		} default: ;
		}
		break;
	case FIR_FLOAT:
		switch (Value.GetType()) {
		case FIR_NIL:
			return 0.0;
		case FIR_BOOL:
			return Value.GetBool() ? 1.0 : 0.0;
		case FIR_INT:
			return (double)Value.GetInt();
		case FIR_FLOAT:
			return Value;
		case FIR_STR: {
			FIRFloat ValFloat = 0.0f;
			FDefaultValueHelper::ParseDouble(Value.GetString(), ValFloat);
			return ValFloat;
		} default: ;
		}
		break;
	case FIR_STR:
		switch (Value.GetType()) {
		case FIR_NIL:
			return FString(TEXT("Nil"));
		case FIR_BOOL:
			return FString(Value.GetBool() ? TEXT("true") : TEXT("false"));
		case FIR_INT:
			return FString::Printf(TEXT("%lld"), Value.GetInt());
		case FIR_FLOAT:
			return FString::Printf(TEXT("%lg"), Value.GetFloat());
		case FIR_STR:
			return Value;
		case FIR_OBJ:
			return FIRObjectToString(Value.GetObj().Get());
		case FIR_CLASS:
			return FIRClassToString(Value.GetClass());
		case FIR_TRACE:
			return FIRObjectToString(*Value.GetTrace());
		default: ;
		}
		break;
	case FIR_OBJ:
		switch (Value.GetType()) {
		case FIR_OBJ:
			return Value;
		case FIR_NIL:
			return FWeakObjectPtr((UObject*)nullptr);
		case FIR_CLASS:
			return FWeakObjectPtr((UObject*)Value.GetClass());
		case FIR_TRACE:
			return FWeakObjectPtr(*Value.GetTrace());
		default: ;
		}
		break;
	case FIR_CLASS:
		switch (Value.GetType()) {
		case FIR_NIL:
			return (UClass*)nullptr;
		case FIR_OBJ:
			return Cast<UClass>(Value.GetObj().Get());
		case FIR_CLASS:
			return Value;
		case FIR_TRACE:
			return Cast<UClass>(*Value.GetTrace());
		default: ;
		}
		break;
	case FIR_TRACE:
		switch (Value.GetType()) {
		case FIR_NIL:
			return FFIRTrace();
		case FIR_OBJ:
			return FFIRTrace(Value.GetObj().Get());
		case FIR_CLASS:
			return FFIRTrace(Value.GetClass());
		case FIR_TRACE:
			return Value;
		default: ;
		}
		break;
	case FIR_STRUCT:
		switch (Value.GetType()) {
		case FIR_STRUCT:
			return Value;
		default: ;
		}
		break;
	case FIR_ARRAY:
		switch (Value.GetType()) {
		case FIR_ARRAY:
		default: ;
		}
		break;
	default: ;
	}
	return FIRAny();
}
