#pragma once

#include "FINFuncProperty.h"
#include "FINBoolProperty.generated.h"

UCLASS(BlueprintType)
class UFINBoolProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UBoolProperty* Property = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(void* Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx);
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(void* Ctx, const FINAny& Value) const override {
		if (Property) Property->SetPropertyValue_InContainer(Ctx, Value.GetBool());
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_BOOL; }
	// End UFINProperty
	
};
