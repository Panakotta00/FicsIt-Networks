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
	EClassCastFlags CastFlags = Prop->GetClass()->ClassCastFlags;
	if (CastFlags & CASTCLASS_UBoolProperty) return FIN_BOOL;
	if (CastFlags & CASTCLASS_UIntProperty) return FIN_INT;
	if (CastFlags & CASTCLASS_UFloatProperty) return FIN_FLOAT;
	if (CastFlags & CASTCLASS_UStrProperty) return FIN_STR;
	if (CastFlags & CASTCLASS_UObjectProperty) return FIN_OBJ;
	if (CastFlags & CASTCLASS_UClassProperty) return FIN_CLASS;
	if (CastFlags & CASTCLASS_UStructProperty) {
		UStructProperty* StructProp = Cast<UStructProperty>(Prop);
		if (StructProp->Struct == FFINNetworkTrace::StaticStruct()) {
			return FIN_TRACE;
		}
		return FIN_STRUCT;
	}
	return FIN_NIL;
}
