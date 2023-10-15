#pragma once

#include "FINFuncProperty.h"
#include "FINFloatProperty.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINFloatProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	FFloatProperty* FloatProperty = nullptr;
	FDoubleProperty* DoubleProperty = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(const FFINExecutionContext& Ctx) const override {
		if (FloatProperty) return FloatProperty->GetPropertyValue_InContainer(Ctx.GetGeneric());
		if (DoubleProperty) return DoubleProperty->GetPropertyValue_InContainer(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFINExecutionContext& Ctx, const FINAny& Value) const override {
		FINFloat Val = 0.0f;
		if (Value.GetType() == FIN_FLOAT) Val = Value.GetFloat();
		else if (Value.GetType() == FIN_INT) Val = Value.GetInt();
		else return;
		if (FloatProperty) FloatProperty->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		if (DoubleProperty) DoubleProperty->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_FLOAT; }
	// End UFINProperty
	
};
