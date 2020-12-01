#pragma once

#include "FINFuncProperty.h"
#include "FINStructProperty.generated.h"

UCLASS(BlueprintType)
class UFINStructProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UStructProperty* Property = nullptr;
	UPROPERTY()
	UScriptStruct* Struct = nullptr;
	
	// Begin UFINProperty
	virtual FINAny GetValue(void* Ctx) const override {
		if (Property) {
			if (Property->Struct == FFINDynamicStructHolder::StaticStruct()) {
				return *Property->ContainerPtrToValuePtr<FFINDynamicStructHolder>(Ctx);
			}
			return FFINDynamicStructHolder(Property->Struct, Property->ContainerPtrToValuePtr<void>(Ctx));
		}
		return Super::GetValue(Ctx);
	}
	
	virtual void SetValue(void* Ctx, const FINAny& Value) const override {
		if (Property) {
			if (Property->Struct == FFINDynamicStructHolder::StaticStruct()) {
				*Property->ContainerPtrToValuePtr<FFINDynamicStructHolder>(Ctx) = Value.GetStruct();
			}
			check(Property->Struct == Value.GetStruct().GetStruct());
			Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Ctx), Value.GetStruct().GetData());
		} else {
			if (Value.GetType() == FIN_STRUCT && (!Struct || Value.GetStruct().GetStruct() == Struct)) Super::SetValue(Ctx, Value);
		}
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_STRUCT; }
	// End UFINProperty

	/**
	 * Returns the filter type of this struct property
	 */
	virtual UScriptStruct* GetInner() const { return Struct; }
};
