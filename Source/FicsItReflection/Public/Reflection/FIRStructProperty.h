#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRStructProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRStructProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FStructProperty* Property = nullptr;
	UPROPERTY()
	UScriptStruct* Struct = nullptr;
	
	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		FIRAny Value;
		if (Property) {
			if (Property->Struct == FFIRInstancedStruct::StaticStruct()) {
				Value = *Property->ContainerPtrToValuePtr<FFIRInstancedStruct>(Ctx.GetGeneric());
			} else {
				Value = FFIRInstancedStruct::Copy(Property->Struct, Property->ContainerPtrToValuePtr<void>(Ctx.GetGeneric()));
			}
		} else {
			Value = Super::GetValue(Ctx);
		}
		if (Struct == FFIRAnyValue::StaticStruct()) {
			return Value.GetStruct().Get<FFIRAnyValue>();
		}
		return Value;
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& InValue) const override {
		FIRAny Value = InValue;
		if (Struct == FFIRAnyValue::StaticStruct()) {
			Value = FIRAny(TFIRInstancedStruct<FFIRAnyValue>(InValue));
		}
		if (Value.GetType() != FIR_STRUCT) return;
		if (Property) {
			if (Property->Struct == FFIRInstancedStruct::StaticStruct()) {
				*Property->ContainerPtrToValuePtr<FFIRInstancedStruct>(Ctx.GetGeneric()) = Value.GetStruct();
			}
			check(Property->Struct == Value.GetStruct().GetStruct());
			Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Ctx.GetGeneric()), Value.GetStruct().GetData());
		} else {
			if (Value.GetType() == FIR_STRUCT && (!Struct || Value.GetStruct().GetStruct() == Struct)) Super::SetValue(Ctx, Value);
		}
	}

	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_STRUCT; }

	virtual bool IsValidValue(const FFIRAnyValue& Value) const override {
		if (Struct == FFIRAnyValue::StaticStruct()) return true;
		return Super::IsValidValue(Value);
	}
	// End UFINProperty

	/**
	 * Returns the filter type of this struct property
	 */
	virtual UScriptStruct* GetInner() const { return Struct; }
};
