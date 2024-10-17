#pragma once

#include "CoreMinimal.h"
#include "FIRUtils.generated.h"

UCLASS()
class FICSITREFLECTION_API UFIRUtils : public UObject {
	GENERATED_BODY()
public:
	static bool CheckIfVarargs(UFIRProperty* Prop);
	static bool SetIfVarargs(UFIRProperty* Prop, const FFIRExecutionContext& Ctx, const TArray<FFIRAnyValue>& Params);
};

UFIRProperty* FIRCreateFIRPropertyFromFProperty(FProperty* Property, FProperty* OverrideProperty, UObject* Outer);
inline UFIRProperty* FIRCreateFIRPropertyFromFProperty(FProperty* Property, UObject* Outer) {
	return FIRCreateFIRPropertyFromFProperty(Property, Property, Outer);
}
