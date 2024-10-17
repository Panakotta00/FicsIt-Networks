#pragma once

#include "CoreMinimal.h"
#include "FIRArrayProperty.h"
#include "FIRFunction.h"
#include "FIRUFunction.generated.h"

UCLASS()
class FICSITREFLECTION_API UFIRUFunction : public UFIRFunction {
	GENERATED_BODY()
public:
	UPROPERTY()
	UFunction* RefFunction = nullptr;
	UPROPERTY()
	UFIRArrayProperty* VarArgsProperty = nullptr;

	// Begin UFIRFunction
	virtual TArray<FFIRAnyValue> Execute(const FFIRExecutionContext& Ctx, const TArray<FFIRAnyValue>& Params) const override;
};
