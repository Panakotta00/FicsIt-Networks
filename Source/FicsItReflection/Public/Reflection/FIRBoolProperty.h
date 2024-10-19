#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRBoolProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRBoolProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FBoolProperty* Property = nullptr;

	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override {
		if (Property) Property->SetPropertyValue_InContainer(Ctx.GetGeneric(), Value.GetBool());
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_BOOL; }
	// End UFINProperty
	
};
