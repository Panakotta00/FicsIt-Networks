#pragma once

#include "FINProperty.h"
#include "FINObjectProperty.generated.h"

UCLASS(BlueprintType)
class UFINObjectProperty : public UFINProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UObjectProperty* Property = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(void* Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx);
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(void* Ctx, const FINAny& Value) const override {
		if (Property) Property->SetPropertyValue_InContainer(Ctx, Value.GetObject().Get());
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_OBJ; }
	// End UFINProperty
	
};
