#pragma once

#include "FINFuncProperty.h"
#include "FINStructProperty.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINStructProperty : public UFINFuncProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	UStructProperty* Property = nullptr;
	UPROPERTY()
	UScriptStruct* Struct = nullptr;
	
	// Begin UFINProperty
	virtual FINAny GetValue(const FFINExecutionContext& Ctx) const override {
		FINAny Value;
		if (Property) {
			if (Property->Struct == FFINDynamicStructHolder::StaticStruct()) {
				Value = *Property->ContainerPtrToValuePtr<FFINDynamicStructHolder>(Ctx.GetGeneric());
			} else {
				Value = FFINDynamicStructHolder::Copy(Property->Struct, Property->ContainerPtrToValuePtr<void>(Ctx.GetGeneric()));
			}
		} else {
			Value = Super::GetValue(Ctx);
		}
		if (Struct == FFINAnyNetworkValue::StaticStruct()) {
			return Value.GetStruct().Get<FFINAnyNetworkValue>();
		}
		return Value;
	}
	
	virtual void SetValue(const FFINExecutionContext& Ctx, const FINAny& InValue) const override {
		FINAny Value = InValue;
		if (Struct == FFINAnyNetworkValue::StaticStruct()) {
			Value = FINAny(TFINDynamicStruct<FFINAnyNetworkValue>(InValue));
		}
		if (Value.GetType() != FIN_STRUCT) return;
		if (Property) {
			if (Property->Struct == FFINDynamicStructHolder::StaticStruct()) {
				*Property->ContainerPtrToValuePtr<FFINDynamicStructHolder>(Ctx.GetGeneric()) = Value.GetStruct();
			}
			check(Property->Struct == Value.GetStruct().GetStruct());
			Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Ctx.GetGeneric()), Value.GetStruct().GetData());
		} else {
			if (Value.GetType() == FIN_STRUCT && (!Struct || Value.GetStruct().GetStruct() == Struct)) Super::SetValue(Ctx, Value);
		}
	}

	virtual TEnumAsByte<EFINNetworkValueType> GetType() const { return FIN_STRUCT; }

	virtual bool IsValidValue(const FFINAnyNetworkValue& Value) const override {
		if (Struct == FFINAnyNetworkValue::StaticStruct()) return true;
		return Super::IsValidValue(Value);
	}
	// End UFINProperty

	/**
	 * Returns the filter type of this struct property
	 */
	virtual UScriptStruct* GetInner() const { return Struct; }
};
