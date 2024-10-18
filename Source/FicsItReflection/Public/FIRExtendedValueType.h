#pragma once

#include "CoreMinimal.h"
#include "FicsItReflection.h"
#include "FIRInstancedStruct.h"
#include "FIRTypes.h"
#include "FIRExtendedValueType.generated.h"

class UFIRStruct;
class UFIRProperty;

USTRUCT()
struct FICSITREFLECTION_API FFIRExtendedValueType {
	GENERATED_BODY()
private:
	UPROPERTY(SaveGame)
	TEnumAsByte<EFIRValueType> Type;

	UPROPERTY(SaveGame)
	UFIRStruct* RefSubType = nullptr;

	UPROPERTY(SaveGame)
	FFIRInstancedStruct SubType;

public:
	FFIRExtendedValueType() : Type(FIR_NIL) {}

	FFIRExtendedValueType(EFIRValueType InType) : Type(InType) {
		check(InType < FIR_OBJ || InType == FIR_ANY);
	}

	FFIRExtendedValueType(EFIRValueType InType, UFIRStruct* InSubRefType) : Type(InType), RefSubType(InSubRefType) {
		check(InType != FIR_ARRAY);
	}

	FFIRExtendedValueType(EFIRValueType InType, const FFIRExtendedValueType& InSubType) : Type(FIR_ARRAY), SubType(InSubType) {
		check(InType == FIR_ARRAY);
	}

	FFIRExtendedValueType(UFIRProperty* Property);

	FFIRExtendedValueType(const FFIRExtendedValueType& Other) : Type(Other.Type) {
		if (Type == FIR_ARRAY) {
			SubType = Other.SubType;
		} else {
			RefSubType = Other.RefSubType;
		}
	}

	FFIRExtendedValueType& operator=(const FFIRExtendedValueType& Other) {
		if (Other.Type == FIR_ARRAY) {
			if (Type != FIR_ARRAY) RefSubType = nullptr;
			SubType = Other.SubType;
		} else {
			if (Type == FIR_ARRAY) SubType = FFIRInstancedStruct();
			RefSubType = Other.RefSubType;
		}
		Type = Other.Type;
		return *this;
	}

	bool Equals(const FFIRExtendedValueType& Other) {
		if (Type != Other.Type) return false;
		if (Type == FIR_ARRAY) return SubType.Get<FFIRExtendedValueType>().Equals(Other.SubType.Get<FFIRExtendedValueType>());
		if (Type >= FIR_OBJ && Type != FIR_ANY) return RefSubType == Other.RefSubType;
		return true;
	}

	bool IsA(const FFIRExtendedValueType& Other) const;

	EFIRValueType GetType() const { return Type; }

	UFIRStruct* GetRefSubType() const {
		check(Type >= FIR_OBJ && Type != FIR_ANY && Type != FIR_ARRAY);
		return RefSubType;
	}

	FFIRExtendedValueType GetSubType() const {
		check(Type == FIR_ARRAY);
		return SubType.Get<FFIRExtendedValueType>();
	}
};

static FORCEINLINE EFIRValueType GetValueTypeFromProp(FProperty* Prop) {
	uint64 CastFlags = Prop->GetClass()->GetCastFlags();
	if (CastFlags & CASTCLASS_FBoolProperty) return FIR_BOOL;
	if (CastFlags & CASTCLASS_FIntProperty) return FIR_INT;
	if (CastFlags & CASTCLASS_FFloatProperty) return FIR_FLOAT;
	if (CastFlags & CASTCLASS_FStrProperty) return FIR_STR;
	if (CastFlags & CASTCLASS_FObjectProperty) return FIR_OBJ;
	if (CastFlags & CASTCLASS_FClassProperty) return FIR_CLASS;
	if (CastFlags & CASTCLASS_FStructProperty) {
		FStructProperty* StructProp = CastField<FStructProperty>(Prop);
		if (StructProp->Struct == FFIRTrace::StaticStruct()) {
			return FIR_TRACE;
		}
		return FIR_STRUCT;
	}
	return FIR_NIL;
}

static FORCEINLINE FString FIRGetNetworkValueTypeName(EFIRValueType InType) {
	switch (InType) {
	case FIR_NIL:
		return TEXT("Nil");
	case FIR_BOOL:
		return TEXT("Bool");
	case FIR_INT:
		return TEXT("Int");
	case FIR_FLOAT:
		return TEXT("Float");
	case FIR_STR:
		return TEXT("String");
	case FIR_OBJ:
		return TEXT("Object");
	case FIR_CLASS:
		return TEXT("Class");
	case FIR_TRACE:
		return TEXT("Trace");
	case FIR_STRUCT:
		return TEXT("Struct");
	case FIR_ARRAY:
		return TEXT("Array");
	case FIR_ANY:
		return TEXT("Any");
	default:
		return TEXT("Unknown");
	}
}
