#pragma once

#include "FINFuncProperty.h"
#include "UObject/UnrealType.h"
#include "FINStrProperty.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINStrProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	FStrProperty* Property = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(const FFINExecutionContext& Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFINExecutionContext& Ctx, const FINAny& Value) const override {
		if (Value.GetType() != FIN_STR) return;
		FString Str = Value.GetString();
		if (Property) Property->SetPropertyValue_InContainer(Ctx.GetGeneric(), Str);
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_STR; }
	// End UFINProperty
	
};
