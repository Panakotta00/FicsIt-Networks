#pragma once

#include "FINProperty.h"
#include "FINTraceProperty.generated.h"

UCLASS(BlueprintType)
class UFINTraceProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UStructProperty* Property = nullptr;
	UPROPERTY()
	UClass* Subclass = nullptr;
	
	// Begin UFINProperty
	virtual FINAny GetValue(const FFINExecutionContext& Ctx) const override {
		if (Property) return Property->ContainerPtrToValuePtr<FFINNetworkTrace>(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFINExecutionContext& Ctx, const FINAny& Value) const override {
		if (Value.GetType() != FIN_TRACE) return;
		UObject* Obj = Value.GetTrace().GetUnderlyingPtr().Get();
		if (Obj && Subclass && !Obj->IsA(GetSubclass())) return;
		if (Property) *Property->ContainerPtrToValuePtr<FFINNetworkTrace>(Ctx.GetGeneric()) = Value.GetTrace();
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_TRACE; }
	// End UFINProperty

	/**
	 * Returns the subclass which all values need to be child of (or equal to)
	 * if they want to set the value.
	 * Nullptr if any class is allows
	 */
	virtual UClass* GetSubclass() const { return Subclass; }
};
