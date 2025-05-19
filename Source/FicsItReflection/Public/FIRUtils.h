#pragma once

#include "CoreMinimal.h"
#include "FIRFunction.h"
#include "FIRProperty.h"
#include "FIRSignal.h"
#include "FIRUtils.generated.h"

UCLASS()
class FICSITREFLECTION_API UFIRUtils : public UObject {
	GENERATED_BODY()
public:
	static bool CheckIfVarargs(UFIRProperty* Prop);
	static bool SetIfVarargs(UFIRProperty* Prop, const FFIRExecutionContext& Ctx, const TArray<FFIRAnyValue>& Params);
};

UFIRProperty* FIRCreateFIRPropertyFromFProperty(FProperty* Property, FProperty* OverrideProperty, UObject* Outer, FName Name);
inline UFIRProperty* FIRCreateFIRPropertyFromFProperty(FProperty* Property, UObject* Outer, FName Name) {
	return FIRCreateFIRPropertyFromFProperty(Property, Property, Outer, Name);
}

FName FIRFunctionObjectName(EFIRFunctionFlags flags, const FString& InternalName);
FName FIRPropertyObjectName(EFIRPropertyFlags flags, const FString& InternalName);
FName FIRSignalObjectName(EFIRSignalFlags flags, const FString& InternalName);