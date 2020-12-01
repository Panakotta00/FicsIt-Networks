#pragma once

#include "CoreMinimal.h"
#include "FINBase.h"
#include "Network/FINAnyNetworkValue.h"
#include "Network/FINNetworkValues.h"
#include "FINExecutionContext.h"
#include "FINProperty.generated.h"

UENUM(BlueprintType)
enum EFINRepPropertyFlags {
	FIN_Prop_None			= 0b000000000,
	FIN_Prop_Attrib			= 0b000000001,
	FIN_Prop_ReadOnly		= 0b000000010,
	FIN_Prop_Param			= 0b000000100,
	FIN_Prop_OutParam		= 0b000001000,
	FIN_Prop_RetVal			= 0b000010000,
	FIN_Prop_Runtime		= 0b011100000,
	FIN_Prop_RT_Sync		= 0b000100000,
	FIN_Prop_RT_Parallel	= 0b001000000,
	FIN_Prop_RT_Async		= 0b010000000,
	FIN_Prop_Sync			= 0b000100000,
	FIN_Prop_Parallel		= 0b001100000,
	FIN_Prop_Async			= 0b011100000,
	FIN_Prop_ClassProp		= 0b100000000,
};

inline EFINRepPropertyFlags operator|(EFINRepPropertyFlags Flags1, EFINRepPropertyFlags Flags2) {
	return (EFINRepPropertyFlags)((uint16)Flags1 | (uint16)Flags2);
}

inline EFINRepPropertyFlags operator&(EFINRepPropertyFlags Flags1, EFINRepPropertyFlags Flags2) {
	return (EFINRepPropertyFlags)(((uint16)Flags1) & ((uint16)Flags2));
}

inline EFINRepPropertyFlags operator~(EFINRepPropertyFlags Flags) {
	return (EFINRepPropertyFlags)~(uint16)Flags;
}

class UFINProperty;

USTRUCT()
struct FFINPropertyGetterFunc {
	GENERATED_BODY()

	UPROPERTY()
	UFunction* Function = nullptr;
	UPROPERTY()
	UFINProperty* Property = nullptr;
	TFunction<FINAny(const FFINExecutionContext&)> GetterFunc;

	FINAny operator()(const FFINExecutionContext& Ctx, bool* Done = nullptr) const;
};

USTRUCT()
struct FFINPropertySetterFunc {
	GENERATED_BODY()

	UPROPERTY()
	UFunction* Function = nullptr;
	UPROPERTY()
	UFINProperty* Property = nullptr;
	TFunction<void(const FFINExecutionContext&, const FINAny&)> SetterFunc;

	bool operator()(const FFINExecutionContext& Ctx, const FINAny& Any) const;
};

UCLASS(BlueprintType)
class UFINProperty : public UFINBase {
	GENERATED_BODY()
public:
	EFINRepPropertyFlags PropertyFlags = FIN_Prop_Sync;
	
	/**
	 * Returns the data type of the property
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_NIL; }

	/**
	 * Returns the property type
	 */
	virtual EFINRepPropertyFlags GetPropertyFlags() const { return PropertyFlags; }

	/**
	 * Sets the property value in the given container.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void SetValue(UObject* Ctx, const FFINAnyNetworkValue& Value) const { SetValue((void*)Ctx, Value); }
	virtual void SetValue(void* Ctx, const FINAny& Value) const {}

	/**
	 * Gets the property value in the given container.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    FFINAnyNetworkValue GetValue(UObject* Ctx) const { return GetValue((void*)Ctx); }
    virtual FINAny GetValue(void* Ctx) const { return FINAny(); }

	/**
	 * Checks if the given value is valid for SetValue
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual bool IsValidValue(const FFINAnyNetworkValue& Value) const { return Value.GetType() == GetType(); }
};


inline FINAny FFINPropertyGetterFunc::operator()(const FFINExecutionContext& Ctx, bool* Done) const {
	if (Function) {
		if (Done) *Done = true;
		uint8* Params = (uint8*)FMemory::Malloc(Function->PropertiesSize);
		FMemory::Memzero(Params + Function->ParmsSize, Function->PropertiesSize - Function->ParmsSize);
		Function->InitializeStruct(Params);
		for (UProperty* LocalProp = Function->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Params);
		}
		Ctx.GetObject()->ProcessEvent(Function, Params);
		FINAny Return = Property->GetValue(Params);
		for (UProperty* P = Function->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(Function->ParmsSize)) {
				P->DestroyValue_InContainer(Params);
			}
		}
		FMemory::Free(Params);
		return Return;
	}
	if (GetterFunc) {
		if (Done) *Done = true;
		return GetterFunc(Ctx);
	}
	if (Done) *Done = false;
	return FINAny();
}

inline bool FFINPropertySetterFunc::operator()(const FFINExecutionContext& Ctx, const FINAny& Any) const {
	if (Function) {
		uint8* Params = (uint8*)FMemory::Malloc(Function->PropertiesSize);
		FMemory::Memzero(Params + Function->ParmsSize, Function->PropertiesSize - Function->ParmsSize);
		Function->InitializeStruct(Params);
		for (UProperty* LocalProp = Function->FirstPropertyToInit; LocalProp != NULL; LocalProp = (UProperty*)LocalProp->Next) {
			LocalProp->InitializeValue_InContainer(Params);
		}
		Property->SetValue(Params, Any);
		Ctx.GetObject()->ProcessEvent(Function, Params);
		for (UProperty* P = Function->DestructorLink; P; P = P->DestructorLinkNext) {
			if (!P->IsInContainer(Function->ParmsSize)) {
				P->DestroyValue_InContainer(Params);
			}
		}
		FMemory::Free(Params);
		return true;
	}
	if (SetterFunc) {
		SetterFunc(Ctx, Any);
		return true;
	}
	return false;
}	