#pragma once

#include "FINFuncProperty.h"
#include "FINStrProperty.generated.h"

UCLASS(BlueprintType)
class UFINStrProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UStrProperty* Property = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(void* Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx);
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(void* Ctx, const FINAny& Value) const override {
		if (Property) Property->SetPropertyValue_InContainer(Ctx, Value.GetString());
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_STR; }
	// End UFINProperty
	
};
