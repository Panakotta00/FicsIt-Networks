#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRTraceProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRTraceProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FStructProperty* Property = nullptr;
	UPROPERTY()
	UClass* Subclass = nullptr;
	
	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		if (Property) return *Property->ContainerPtrToValuePtr<FFIRTrace>(Ctx.GetGeneric());
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override {
		if (Value.GetType() != FIR_TRACE) return;
		UObject* Obj = Value.GetTrace().GetUnderlyingPtr();
		if (IsValid(Obj) && Subclass && !Obj->IsA(GetSubclass())) return;
		if (Property) *Property->ContainerPtrToValuePtr<FFIRTrace>(Ctx.GetGeneric()) = Value.GetTrace();
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_TRACE; }
	// End UFINProperty

	/**
	 * Returns the subclass which all values need to be child of (or equal to)
	 * if they want to set the value.
	 * Nullptr if any class is allows
	 */
	virtual UClass* GetSubclass() const { return Subclass; }
};
