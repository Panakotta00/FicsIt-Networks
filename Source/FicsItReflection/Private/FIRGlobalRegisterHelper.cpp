#include "FIRGlobalRegisterHelper.h"

FFIRGlobalRegisterHelper FFIRGlobalRegisterHelper::Instance;

void FFIRGlobalRegisterHelper::Register() {
	TArray<int> priorities;
	Get().FunctionsToCall.GetKeys(priorities);
	priorities.Sort();
	Algo::Reverse(priorities);
	for (int priority : priorities) {
		for (const RegisterFunction& Func : Get().FunctionsToCall[priority]) {
			Func();
		}
	}
	Get().FunctionsToCall.Empty();
}

void FFIRGlobalRegisterHelper::AddFunction(const RegisterFunction& Func, int Priority) {
	Get().FunctionsToCall.FindOrAdd(Priority).Add(Func);
}
