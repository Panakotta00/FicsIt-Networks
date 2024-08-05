#pragma once

#include "CoreMinimal.h"

class FICSITNETWORKS_API FFINGlobalRegisterHelper {
public:
	typedef TFunction<void()> RegisterFunction;
private:
 	TMap<int, TArray<RegisterFunction>> FunctionsToCall;

	FFINGlobalRegisterHelper() = default;

	static FFINGlobalRegisterHelper Instance;
public:
	static FFINGlobalRegisterHelper& Get() { return Instance; }

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

struct FICSITNETWORKS_API FFINStaticGlobalRegisterFunc {
	FFINStaticGlobalRegisterFunc(const FFINGlobalRegisterHelper::RegisterFunction& Func, int Priority = 0) {
		FFINGlobalRegisterHelper::AddFunction(Func, Priority);
	}
};

FORCEINLINE bool operator==(const FFINGlobalRegisterHelper::RegisterFunction& A, const FFINGlobalRegisterHelper::RegisterFunction& B) {
	return TFunction<void()>(A) == TFunction<void()>(B);
}
