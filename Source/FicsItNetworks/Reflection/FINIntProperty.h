#pragma once

#include "FINFuncProperty.h"
#include "FINIntProperty.generated.h"

UCLASS(BlueprintType)
class UFINIntProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UIntProperty* Property = nullptr;
	UPROPERTY()
	UInt64Property* Property64 = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(const FFINExecutionContext& Ctx) const override {
		if (Property) return (FINInt)Property->GetPropertyValue_InContainer(Ctx.GetGeneric());
		else if (Property64) return Property64->GetPropertyValue_InContainer(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFINExecutionContext& Ctx, const FINAny& Value) const override {
		FINInt Val = 0;
		if (Value.GetType() == FIN_INT) Val = Value.GetInt();
		else if (Value.GetType() == FIN_FLOAT) Val = Value.GetFloat();
		else return;
		if (Property) Property->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		else if (Property64) Property64->SetPropertyValue_InContainer(Ctx.GetGeneric(), Val);
		return Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_INT; }
	// End UFINProperty
	
};
