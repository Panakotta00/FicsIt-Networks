#pragma once

#include "CoreMinimal.h"

class FICSITREFLECTION_API FFIRGlobalRegisterHelper {
public:
	typedef TFunction<void()> RegisterFunction;
private:
 	TMap<int, TArray<RegisterFunction>> FunctionsToCall;

	FFIRGlobalRegisterHelper() = default;

	static FFIRGlobalRegisterHelper Instance;
public:
	static FFIRGlobalRegisterHelper& Get() { return Instance; }

	/**
	 * Calls all register function and clears the function list afterwards
	 */
	static void Register();

	/**
	 * Adds the given register functions to the list of functions
	 * that need to get called when register runs.
	 *
	 * @param[in]	Func	the function you want to add to the list
	 */
	static void AddFunction(const RegisterFunction& Func, int Priority);
};

struct FICSITREFLECTION_API FFIRStaticGlobalRegisterFunc {
	FFIRStaticGlobalRegisterFunc(const FFIRGlobalRegisterHelper::RegisterFunction& Func, int Priority = 0) {
		FFIRGlobalRegisterHelper::AddFunction(Func, Priority);
	}
};
