#pragma once

#include "CoreMinimal.h"
#include "FIRAnyValue.h"
#include "FIRBase.h"
#include "FIRExecutionContext.h"
#include "FIRTypes.h"
#include "FIRProperty.generated.h"

UENUM(BlueprintType)
enum EFIRPropertyFlags {
	FIR_Prop_None			= 0,
	FIR_Prop_Attrib			= 0b00000000001,
	FIR_Prop_ReadOnly		= 0b00000000010,
	FIR_Prop_Param			= 0b00000000100,
	FIR_Prop_OutParam		= 0b00000001000,
	FIR_Prop_RetVal			= 0b00000010000,
	FIR_Prop_Runtime		= 0b00011100000,
	FIR_Prop_RT_Sync		= 0b00000100000,
	FIR_Prop_RT_Parallel	= 0b00001000000,
	FIR_Prop_RT_Async		= 0b00010000000,
	FIR_Prop_Sync			= 0b00000100000,
	FIR_Prop_Parallel		= 0b00001100000,
	FIR_Prop_Async			= 0b00011100000,
	FIR_Prop_ClassProp		= 0b00100000000,
	FIR_Prop_StaticSource	= 0b01000000000,
	FIR_Prop_StaticProp		= 0b10000000000,
};

ENUM_CLASS_FLAGS(EFIRPropertyFlags)

class UFIRProperty;

USTRUCT()
struct FICSITREFLECTION_API FFIRPropertyGetterFunc {
	GENERATED_BODY()

	UPROPERTY()
	UFunction* Function = nullptr;
	UPROPERTY()
	UFIRProperty* Property = nullptr;
	TFunction<FIRAny(const FFIRExecutionContext&)> GetterFunc;

	FIRAny operator()(const FFIRExecutionContext& Ctx, bool* Done = nullptr) const;
};

USTRUCT()
struct FICSITREFLECTION_API FFIRPropertySetterFunc {
	GENERATED_BODY()

	UPROPERTY()
	UFunction* Function = nullptr;
	UPROPERTY()
	UFIRProperty* Property = nullptr;
	TFunction<void(const FFIRExecutionContext&, const FIRAny&)> SetterFunc;

	bool operator()(const FFIRExecutionContext& Ctx, const FIRAny& Any) const;
};

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRProperty : public UFIRBase {
	GENERATED_BODY()
public:
	EFIRPropertyFlags PropertyFlags = FIR_Prop_Sync;
	
	/**
	 * Returns the data type of the property
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_NIL; }

	/**
	 * Returns the property type
	 */
	virtual EFIRPropertyFlags GetPropertyFlags() const { return PropertyFlags; }

	/**
	 * Sets the property value in the given container.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	void SetValue(UObject* Ctx, const FFIRAnyValue& Value) const { SetValue(FFIRExecutionContext(Ctx), Value); }
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const {}

	/**
	 * Gets the property value in the given container.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    FFIRAnyValue GetValue(UObject* Ctx) const { return GetValue(FFIRExecutionContext(Ctx)); }
    virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const { return FIRAny(); }

	/**
	 * Checks if the given value is valid for SetValue
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual bool IsValidValue(const FFIRAnyValue& Value) const { return Value.GetType() == GetType(); }
};
