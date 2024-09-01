#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRIntProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRIntProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FIntProperty* Property = nullptr;
	FInt64Property* Property64 = nullptr;
	FByteProperty* Property8 = nullptr;

	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		if (Property) return (FIRInt)Property->GetPropertyValue_InContainer(Ctx.GetGeneric());
		if (Property64) return Property64->GetPropertyValue_InContainer(Ctx.GetGeneric());
		if (Property8) return (FIRInt)Property8->GetPropertyValue_InContainer(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override {
		FIRInt Val = 0;
		if (Value.GetType() == FIR_INT) Val = Value.GetInt();
		else if (Value.GetType() == FIR_FLOAT) Val = Value.GetFloat();
		else return;
		if (Property) Property->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		else if (Property64) Property64->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		else if (Property8) Property8->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		return Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_INT; }
	// End UFINProperty
	
};
