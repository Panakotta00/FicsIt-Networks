#include "FINGlobalRegisterHelper.h"

FFINGlobalRegisterHelper FFINGlobalRegisterHelper::Instance;

void FFINGlobalRegisterHelper::Register() {
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

void FFINGlobalRegisterHelper::AddFunction(const RegisterFunction& Func, int Priority) {
	Get().FunctionsToCall.FindOrAdd(Priority).Add(Func);
}
