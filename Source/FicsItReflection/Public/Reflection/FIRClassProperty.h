#pragma once

#include "CoreMinimal.h"
#include "FIRFuncProperty.h"
#include "FIRClassProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRClassProperty : public UFIRFuncProperty {
	GENERATED_BODY()
public:
	FClassProperty* Property = nullptr;
	UPROPERTY()
	UClass* Subclass = nullptr;

	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		if (Property) return FWeakObjectPtr(Property->GetPropertyValue_InContainer(Ctx.GetGeneric()));
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override {
		UClass* Class = nullptr;
		if (Value.GetType() == FIR_CLASS) Class = Value.GetClass();
		else if (Value.GetType() == FIR_OBJ) Class = Cast<UClass>(Value.GetObj().Get());
		else if (Class) return;
		if (Class && GetSubclass() && !Class->IsChildOf(GetSubclass())) return;
		if (Property) Property->SetPropertyValue_InContainer(Ctx.GetGeneric(), Class);
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFIRValueType> GetType() const { return FIR_CLASS; }
	// End UFINProperty

	/**
	 * Returns the subclass which all values need to be child of (or equal to)
	 * if they want to set the value.
	 * Nullptr if any class is allows
	 */
	virtual UClass* GetSubclass() const {
		if (Subclass) return Subclass;
		if (Property) return Property->PropertyClass;
		return nullptr;
	}
};
