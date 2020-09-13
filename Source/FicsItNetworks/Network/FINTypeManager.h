#pragma once

#include "CoreMinimal.h"

#include "FINNetworkValues.h"

#include "FINTypeManager.generated.h"

/**
 * Struct holding information about a single parameter of a function
 */
USTRUCT()
struct FFINFunctionParameter {
	GENERATED_BODY()
	
	EFINNetworkValueType Type;
	FString Name;
	bool bOutputValue;

	FFINFunctionParameter() = default;
	FFINFunctionParameter(UProperty* Prop);
};

/**
 * Base struct for all function available to the network
 */
USTRUCT()
struct FFINFunction {
	GENERATED_BODY()
	
	virtual ~FFINFunction() = default;

	/**
	 * Returns the name of the function
	 */
	virtual FString GetName() { return "Undefined"; }

	/**
	 * Returns the paramters/signature of the function
	 */
	virtual TArray<FFINFunctionParameter> GetParameters() { return TArray<FFINFunctionParameter>(); }

	/**
	 * Returns the description of the function
	 */
	virtual FString GetDescription() { return ""; }
};

/**
 * Function struct for all ufunction based functions in the network.
 */
USTRUCT()
struct FFINUFunction : public FFINFunction {
	GENERATED_BODY()
private:
	UFunction* Func = nullptr;

public:
	FFINUFunction() = default;
	FFINUFunction(UFunction* Func) : Func(Func) { check(IsNetworkFunction(Func)); }

	// Begin FFINFunction
	virtual FString GetName() override;
	virtual TArray<FFINFunctionParameter> GetParameters() override;
	virtual FString GetDescription() override;
	// End FFINFunction

	/**
	 * Returns the ufunction this struct represents
	 */
	UFunction* GetUFunc() const;

	/**
	 * Checks if the give UFunctions is a valid network function
	 */
	static bool IsNetworkFunction(UFunction* Func);
};

/**
 * Struct representing a network type
 */
USTRUCT()
struct FFINType {
	GENERATED_BODY()

	virtual ~FFINType() = default;
	virtual FString GetName() { return "Undefined"; }
	virtual TArray<TSharedRef<FFINFunction>> GetFunctions() { return TArray<TSharedRef<FFINFunction>>(); }
};

/**
 * Struct representing a UClass based network type
 */
USTRUCT()
struct FFINUType : public FFINType {
	GENERATED_BODY()
private:
	UClass* Type = nullptr;

public:
	FFINUType() = default;
	FFINUType(UClass* Type) : Type(Type) {}

	// Begin FFINType
	virtual FString GetName() override;
	virtual TArray<TSharedRef<FFINFunction>> GetFunctions() override;
	// End FFINType
};

USTRUCT()
struct FFINTypeManager {
	GENERATED_BODY()
public:
	/**
	 * Searches in the class for functions implemented by this exact type
	 * (excluding parents)
	 */
	static TArray<TSharedRef<FFINFunction>> FindFunctionsOfType(UClass* Type);
};
