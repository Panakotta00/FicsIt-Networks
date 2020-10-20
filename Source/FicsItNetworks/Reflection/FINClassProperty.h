#pragma once

#include "FINProperty.h"
#include "FINClassProperty.generated.h"

UCLASS(BlueprintType)
class UFINClassProperty : public UFINProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UClassProperty* Property = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(void* Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx);
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(void* Ctx, const FINAny& Value) const override {
		if (Property) Property->SetPropertyValue_InContainer(Ctx, Value.GetClass());
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_CLASS; }
	// End UFINProperty
	
};
