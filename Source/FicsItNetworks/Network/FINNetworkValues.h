#pragma once

#include "CoreMinimal.h"

#include "FINNetworkTrace.h"
#include "FINDynamicStructHolder.h"
#include "FINNetworkValues.generated.h"

struct FFINAnyNetworkValue;

UENUM(BlueprintType)
enum EFINNetworkValueType {
	FIN_NIL = 0,
	FIN_BOOL,
	FIN_INT,
	FIN_FLOAT,
	FIN_STR,
	FIN_OBJ,
	FIN_CLASS,
	FIN_TRACE,
	FIN_STRUCT,
	FIN_ARRAY,
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
typedef FFINAnyNetworkValue FINAny;
typedef TArray<FINAny> FINArray;

static inline EFINNetworkValueType GetValueTypeFromProp(UProperty* Prop) {
	uint64 CastFlags = Prop->GetClass()->GetCastFlags();
	if (CastFlags & CASTCLASS_FBoolProperty) return FIN_BOOL;
	if (CastFlags & CASTCLASS_FIntProperty) return FIN_INT;
	if (CastFlags & CASTCLASS_FFloatProperty) return FIN_FLOAT;
	if (CastFlags & CASTCLASS_FStrProperty) return FIN_STR;
	if (CastFlags & CASTCLASS_FObjectProperty) return FIN_OBJ;
	if (CastFlags & CASTCLASS_FClassProperty) return FIN_CLASS;
	if (CastFlags & CASTCLASS_FStructProperty) {
		UStructProperty* StructProp = Cast<UStructProperty>(Prop);
		if (StructProp->Struct == FFINNetworkTrace::StaticStruct()) {
			return FIN_TRACE;
		}
		return FIN_STRUCT;
	}
	return FIN_NIL;
}
