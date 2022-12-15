#pragma once

#include "FINBase.h"
#include "FINExecutionContext.h"
#include "Network/FINAnyNetworkValue.h"
#include "Network/FINNetworkValues.h"
#include "FINProperty.generated.h"

UENUM(BlueprintType)
enum EFINRepPropertyFlags {
	FIN_Prop_None			= 0,
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

ENUM_CLASS_FLAGS(EFINRepPropertyFlags)

class UFINProperty;

USTRUCT()
struct FICSITNETWORKS_API FFINPropertyGetterFunc {
	GENERATED_BODY()

	UPROPERTY()
	UFunction* Function = nullptr;
	UPROPERTY()
	UFINProperty* Property = nullptr;
	TFunction<FINAny(const FFINExecutionContext&)> GetterFunc;

	FINAny operator()(const FFINExecutionContext& Ctx, bool* Done = nullptr) const;
};

USTRUCT()
struct FICSITNETWORKS_API FFINPropertySetterFunc {
	GENERATED_BODY()

	UPROPERTY()
	UFunction* Function = nullptr;
	UPROPERTY()
	UFINProperty* Property = nullptr;
	TFunction<void(const FFINExecutionContext&, const FINAny&)> SetterFunc;

	bool operator()(const FFINExecutionContext& Ctx, const FINAny& Any) const;
};

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINProperty : public UFINBase {
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
	void SetValue(UObject* Ctx, const FFINAnyNetworkValue& Value) const { SetValue(FFINExecutionContext(Ctx), Value); }
	virtual void SetValue(const FFINExecutionContext& Ctx, const FINAny& Value) const {}

	/**
	 * Gets the property value in the given container.
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    FFINAnyNetworkValue GetValue(UObject* Ctx) const { return GetValue(FFINExecutionContext(Ctx)); }
    virtual FINAny GetValue(const FFINExecutionContext& Ctx) const { return FINAny(); }

	/**
	 * Checks if the given value is valid for SetValue
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	virtual bool IsValidValue(const FFINAnyNetworkValue& Value) const { return Value.GetType() == GetType(); }
};
