#pragma once

#include "FIRTrace.h"
#include "FIRInstancedStruct.h"
#include "FIRTypes.generated.h"

struct FFIRAnyValue;

UENUM(BlueprintType)
enum EFIRValueType : int {
	FIR_NIL = 0,
	FIR_BOOL,
	FIR_INT,
	FIR_FLOAT,
	FIR_STR,
	FIR_OBJ,
	FIR_CLASS,
	FIR_TRACE,
	FIR_STRUCT,
	FIR_ARRAY,
	FIR_ANY,
};

typedef bool FIRBool;
typedef int64 FIRInt;
typedef double FIRFloat;
typedef FString FIRStr;
typedef FWeakObjectPtr FIRObj;
typedef UClass* FIRClass;
typedef FFIRTrace FIRTrace;
typedef FFIRInstancedStruct FIRStruct;
typedef FFIRAnyValue FIRAny;
typedef TArray<FIRAny> FIRArray;
