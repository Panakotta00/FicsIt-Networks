#pragma once

#include "CoreMinimal.h"
#include "FIRProperty.h"
#include "FIRFuncProperty.generated.h"

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRFuncProperty : public UFIRProperty {
	GENERATED_BODY()
public:
	UPROPERTY()
	FFIRPropertyGetterFunc GetterFunc;
	UPROPERTY()
	FFIRPropertySetterFunc SetterFunc;

	// Begin UFINProperty
	virtual FIRAny GetValue(const FFIRExecutionContext& Ctx) const override {
		return GetterFunc(Ctx);
	}
	
	virtual void SetValue(const FFIRExecutionContext& Ctx, const FIRAny& Value) const override {
		auto _ = SetterFunc(Ctx, Value);
	}
	// End UFINProperty
};
