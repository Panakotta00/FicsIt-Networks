#pragma once

#include "FINArrayProperty.h"
#include "FINFunction.h"
#include "FINUFunction.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINUFunction : public UFINFunction {
	GENERATED_BODY()
public:
	UPROPERTY()
	UFunction* RefFunction = nullptr;
	UPROPERTY()
	UFINArrayProperty* VarArgsProperty = nullptr;

	// Begin UFINFunction
	virtual TArray<FFINAnyNetworkValue> Execute(const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params) const override;
};
