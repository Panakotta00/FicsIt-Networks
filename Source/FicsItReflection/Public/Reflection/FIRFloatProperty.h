#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRFloatProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRFloatProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FFloatProperty* FloatProperty = nullptr;
	FDoubleProperty* DoubleProperty = nullptr;

	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		if (FloatProperty) return FloatProperty->GetPropertyValue_InContainer(Ctx.GetGeneric());
		if (DoubleProperty) return DoubleProperty->GetPropertyValue_InContainer(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override {
		FIRFloat Val = 0.0f;
		if (Value.GetType() == FIR_FLOAT) Val = Value.GetFloat();
		else if (Value.GetType() == FIR_INT) Val = Value.GetInt();
		else return;
		if (FloatProperty) FloatProperty->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		if (DoubleProperty) DoubleProperty->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_FLOAT; }
	// End UFINProperty
	
};
