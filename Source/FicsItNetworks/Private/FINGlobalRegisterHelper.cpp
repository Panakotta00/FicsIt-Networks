#include "FINGlobalRegisterHelper.h"

FFINGlobalRegisterHelper FFINGlobalRegisterHelper::Instance;

void FFINGlobalRegisterHelper::Register() {
	for (const RegisterFunction& Func : Get().FunctionsToCall) {
		Func();
	}
	Get().FunctionsToCall.Empty();
}

void FFINGlobalRegisterHelper::AddFunction(const RegisterFunction& Func) {
	Get().FunctionsToCall.Add(Func);
}
