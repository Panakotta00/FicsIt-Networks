#pragma once

#include "FINProperty.h"
#include "FINFloatProperty.generated.h"

UCLASS(BlueprintType)
class UFINFloatProperty : public UFINProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UFloatProperty* Property = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(void* Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx);
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(void* Ctx, const FINAny& Value) const override {
		if (Property) Property->SetPropertyValue_InContainer(Ctx, Value.GetFloat());
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_FLOAT; }
	// End UFINProperty
	
};
