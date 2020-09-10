#pragma once

#include "CoreMinimal.h"

#include "FINNetworkTrace.h"
#include "FINDynamicStructHolder.h"
#include "FINNetworkValues.generated.h"

UENUM(BlueprintType)
enum EFINNetworkValueType {
	FIN_NIL,
	FIN_BOOL,
	FIN_INT,
	FIN_FLOAT,
	FIN_STR,
	FIN_OBJ,
	FIN_CLASS,
	FIN_TRACE,
	FIN_STRUCT,
	FIN_ANY,
};

typedef bool FINBool;
typedef int64 FINInt;
typedef double FINFloat;
typedef FString FINStr;
typedef FWeakObjectPtr FINObj;
typedef UClass* FINClass;
typedef FFINNetworkTrace FINTrace;
typedef FFINDynamicStructHolder FINStruct;
