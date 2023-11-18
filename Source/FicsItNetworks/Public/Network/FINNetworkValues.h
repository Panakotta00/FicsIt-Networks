#pragma once

#include "FINNetworkTrace.h"
#include "FINDynamicStructHolder.h"
#include "Misc/EnumRange.h"
#include "FINNetworkValues.generated.h"

struct FFINAnyNetworkValue;
class UFINStruct;
class UFINProperty;

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
ENUM_RANGE_BY_COUNT(EFINNetworkValueType, FIN_ANY + 1);

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

USTRUCT()
struct FICSITNETWORKS_API FFINExpandedNetworkValueType {
	GENERATED_BODY()
private:
	EFINNetworkValueType Type;
	union {
		UFINStruct* RefSubType;
		FFINExpandedNetworkValueType* SubType;
	};

public:
	FFINExpandedNetworkValueType() : Type(FIN_NIL), SubType(nullptr) {}

	FFINExpandedNetworkValueType(EFINNetworkValueType InType) : Type(InType), SubType(nullptr) {
		check(InType < FIN_OBJ || InType == FIN_ANY);
	}

	FFINExpandedNetworkValueType(EFINNetworkValueType InType, UFINStruct* InSubRefType) : Type(InType), RefSubType(InSubRefType) {
		check(InType != FIN_ARRAY);
	}

	FFINExpandedNetworkValueType(EFINNetworkValueType InType, const FFINExpandedNetworkValueType& InSubType) : Type(FIN_ARRAY), SubType(new FFINExpandedNetworkValueType(InSubType)) {
		check(InType == FIN_ARRAY);
	}

	FFINExpandedNetworkValueType(UFINProperty* Property);

	FFINExpandedNetworkValueType(const FFINExpandedNetworkValueType& Other) : Type(Other.Type) {
		if (Type == FIN_ARRAY) {
			SubType = new FFINExpandedNetworkValueType(*Other.SubType);
		} else {
			RefSubType = Other.RefSubType;
		}
	}

	~FFINExpandedNetworkValueType() {
		if (Type == FIN_ARRAY) {
			delete SubType;
		}
	}

	FFINExpandedNetworkValueType& operator=(const FFINExpandedNetworkValueType& Other) {
		if (Other.Type == FIN_ARRAY) {
			if (Type == FIN_ARRAY) *SubType = *Other.SubType;
			else SubType = new FFINExpandedNetworkValueType(*Other.SubType);
		} else {
			if (Type == FIN_ARRAY) delete SubType;
			RefSubType = Other.RefSubType;
		}
		Type = Other.Type;
		return *this;
	}

	bool Equals(const FFINExpandedNetworkValueType& Other) {
		if (Type != Other.Type) return false;
		if (Type == FIN_ARRAY) return SubType->Equals(*Other.SubType);
		if (Type >= FIN_OBJ && Type != FIN_ANY) return RefSubType == Other.RefSubType;
		return true;
	}

	bool IsA(const FFINExpandedNetworkValueType& Other) const;

	EFINNetworkValueType GetType() const { return Type; }

	UFINStruct* GetRefSubType() const {
		check(Type >= FIN_OBJ && Type != FIN_ANY && Type != FIN_ARRAY);
		return RefSubType;
	}

	FFINExpandedNetworkValueType GetSubType() const {
		check(Type == FIN_ARRAY);
		return *SubType;
	}
};

static FORCEINLINE EFINNetworkValueType GetValueTypeFromProp(FProperty* Prop) {
	uint64 CastFlags = Prop->GetClass()->GetCastFlags();
	if (CastFlags & CASTCLASS_FBoolProperty) return FIN_BOOL;
	if (CastFlags & CASTCLASS_FIntProperty) return FIN_INT;
	if (CastFlags & CASTCLASS_FFloatProperty) return FIN_FLOAT;
	if (CastFlags & CASTCLASS_FStrProperty) return FIN_STR;
	if (CastFlags & CASTCLASS_FObjectProperty) return FIN_OBJ;
	if (CastFlags & CASTCLASS_FClassProperty) return FIN_CLASS;
	if (CastFlags & CASTCLASS_FStructProperty) {
		FStructProperty* StructProp = CastField<FStructProperty>(Prop);
		if (StructProp->Struct == FFINNetworkTrace::StaticStruct()) {
			return FIN_TRACE;
		}
		return FIN_STRUCT;
	}
	return FIN_NIL;
}

static FORCEINLINE FString FINGetNetworkValueTypeName(EFINNetworkValueType InType) {
	switch (InType) {
	case FIN_NIL:
		return TEXT("Nil");
	case FIN_BOOL:
		return TEXT("Bool");
	case FIN_INT:
		return TEXT("Int");
	case FIN_FLOAT:
		return TEXT("Float");
	case FIN_STR:
		return TEXT("String");
	case FIN_OBJ:
		return TEXT("Object");
	case FIN_CLASS:
		return TEXT("Class");
	case FIN_TRACE:
		return TEXT("Trace");
	case FIN_STRUCT:
		return TEXT("Struct");
	case FIN_ARRAY:
		return TEXT("Array");
	case FIN_ANY:
		return TEXT("Any");
	default:
		return TEXT("Unknown");
	}
}
