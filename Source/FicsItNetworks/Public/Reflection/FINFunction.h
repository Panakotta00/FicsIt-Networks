#pragma once

#include "FINExecutionContext.h"
#include "FINProperty.h"
#include "FINReflectionException.h"
#include "FINFunction.generated.h"

enum EFINFunctionFlags {
	FIN_Func_None			= 0b00000000,
	FIN_Func_VarArgs		= 0b00000001,
	FIN_Func_Runtime		= 0b00001110,
	FIN_Func_RT_Sync		= 0b00000010,
	FIN_Func_RT_Parallel	= 0b00000100,
	FIN_Func_RT_Async		= 0b00001000,
	FIN_Func_Sync			= 0b00000010,
	FIN_Func_Parallel		= 0b00000110,
	FIN_Func_Async			= 0b00001110,
	FIN_Func_MemberFunc		= 0b00010000,
	FIN_Func_ClassFunc		= 0b00100000,
	FIN_Func_StaticFunc		= 0b01000000,
	FIN_Func_VarRets		= 0b10000000,
};

ENUM_CLASS_FLAGS(EFINFunctionFlags)

USTRUCT()
struct FICSITNETWORKS_API FFINFunctionBadArgumentException : public FFINReflectionException {
	GENERATED_BODY()

	int ArgumentIndex = 0;

	FFINFunctionBadArgumentException() = default;
	FFINFunctionBadArgumentException(UFINFunction* Func, int ArgumentIndex, const FString& Message) : FFINReflectionException(Cast<UFINBase>(Func), Message), ArgumentIndex(ArgumentIndex) {}
};

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINFunction : public UFINBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<UFINProperty*> Parameters;
	
	TFunction<TArray<FFINAnyNetworkValue>(const FFINExecutionContext&, const TArray<FFINAnyNetworkValue>&)> NativeFunction;

	EFINFunctionFlags FunctionFlags = FIN_Func_Sync;
	
	/**
	 * Returns a list of all the parameters this function has
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual TArray<UFINProperty*> GetParameters() const { return Parameters; }

	/**
	 * Returns the function flags of this function
	 */
	virtual EFINFunctionFlags GetFunctionFlags() const { return FunctionFlags; }

	/**
	 * Executes the function with the given properties and the given Ctx
	 */
	virtual TArray<FFINAnyNetworkValue> Execute(const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params) const {
		if (NativeFunction) return NativeFunction(Ctx, Params);
		return TArray<FFINAnyNetworkValue>();
	}
};
