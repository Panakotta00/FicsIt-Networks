#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRStrProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRStrProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FStrProperty* Property = nullptr;

	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override {
		if (Value.GetType() != FIR_STR) return;
		FString Str = Value.GetString();
		if (Property) Property->SetPropertyValue_InContainer(Ctx.GetGeneric(), Str);
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_STR; }
	// End UFINProperty
	
};
