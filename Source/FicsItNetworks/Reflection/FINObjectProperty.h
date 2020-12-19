#pragma once

#include "FINFuncProperty.h"
#include "FINObjectProperty.generated.h"

UCLASS(BlueprintType)
class UFINObjectProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UObjectProperty* Property = nullptr;
	UPROPERTY()
	UClass* Subclass = nullptr;

	// Begin UFINProperty
	virtual FINAny GetValue(const FFINExecutionContext& Ctx) const override {
		if (Property) return static_cast<FINObj>(Property->GetPropertyValue_InContainer(Ctx.GetGeneric()));
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(const FFINExecutionContext& Ctx, const FINAny& Value) const override {
		UObject* Obj = nullptr;
		if (Value.GetType() == FIN_OBJ) Obj = Value.GetObj().Get();
		else if (Value.GetType() == FIN_TRACE) Obj = Value.GetTrace().Get();
		if (Obj && GetSubclass() && !Obj->IsA(GetSubclass())) return;
		if (Property) Property->SetPropertyValue_InContainer(Ctx.GetGeneric(), Obj);
		else Super::SetValue(Ctx, Value);
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_OBJ; }
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
