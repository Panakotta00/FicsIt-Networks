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
	virtual FINAny GetValue(void* Ctx) const override {
		if (Property) return Property->GetPropertyValue_InContainer(Ctx);
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(void* Ctx, const FINAny& Value) const override {
		UObject* Obj = Value.GetObject().Get();
		if (Obj && GetSubclass() && !Obj->IsA(GetSubclass())) return;
		if (Property) Property->SetPropertyValue_InContainer(Ctx, Obj);
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
