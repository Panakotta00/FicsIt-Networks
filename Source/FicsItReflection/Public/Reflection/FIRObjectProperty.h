#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRObjectProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRObjectProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FObjectProperty* Property = nullptr;
	UPROPERTY()
	UClass* Subclass = nullptr; // TODO: Change to UFINClass

	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		if (Property) return static_cast<FIRObj>(Property->GetPropertyValue_InContainer(Ctx.GetGeneric()));
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override {
		UObject* Obj = nullptr;
		if (Value.GetType() == FIR_OBJ) Obj = Value.GetObj().Get();
		else if (Value.GetType() == FIR_TRACE) Obj = Value.GetTrace().Get();
		if (Obj && GetSubclass() && !Obj->IsA(GetSubclass())) return;
		if (Property) Property->SetPropertyValue_InContainer(Ctx.GetGeneric(), Obj);
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_OBJ; }
	// End UFINProperty

	/**
	 * Returns the subclass all to set objects need to be.
	 * Nullptr if any kind of object is allowed.
	 */
	virtual UClass* GetSubclass() const {
		if (Subclass) return Subclass;
		if (Property) return Property->PropertyClass;
		return nullptr;
	}
};
