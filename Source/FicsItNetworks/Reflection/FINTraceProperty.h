#pragma once

#include "FINProperty.h"
#include "FINTraceProperty.generated.h"

UCLASS(BlueprintType)
class UFINTraceProperty : public UFINProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UStructProperty* Property = nullptr;
	UPROPERTY()
	FFINPropertyGetterFunc GetterFunc;
	UPROPERTY()
	FFINPropertySetterFunc SetterFunc;
	
	// Begin UFINProperty
	virtual FINAny GetValue(void* Ctx) const override {
		if (Property) return Property->ContainerPtrToValuePtr<FFINNetworkTrace>(Ctx);
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(void* Ctx, const FINAny& Value) const override {
		if (Property) *Property->ContainerPtrToValuePtr<FFINNetworkTrace>(Ctx) = Value.GetTrace();
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_TRACE; }
	// End UFINProperty
	
};
