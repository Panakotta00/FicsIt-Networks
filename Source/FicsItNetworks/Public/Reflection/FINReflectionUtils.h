#pragma once

#include "FINStructProperty.h"
#include "FINReflectionUtils.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINReflectionUtils : public UObject {
	GENERATED_BODY()
public:
	static bool CheckIfVarargs(UFINProperty* Prop);
	static bool SetIfVarargs(UFINProperty* Prop, const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params);
};
