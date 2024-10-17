#pragma once

#include "CoreMinimal.h"

class UFILLogContainer;

class FICSITLOGLIBRARY_API FFILLogScope {
public:
	DECLARE_DELEGATE_RetVal(FString, FWhereFunction);
	DECLARE_DELEGATE_RetVal(FString, FStackFunction);

	FFILLogScope(UFILLogContainer* Log, FWhereFunction WhereFunction = FWhereFunction(), FStackFunction StackFunction = FStackFunction()) {
		PreviousLog = GetCurrentLog();
		if (Log) GetCurrentLog() = Log;

		if (WhereFunction.IsBound()) {
			PreviousWhereFunction = GetCurrentWhereFunction();
			GetCurrentWhereFunction() = WhereFunction;
		}

		if (StackFunction.IsBound()) {
			PreviousStackFunction = GetCurrentStackFunction();
			GetCurrentStackFunction() = WhereFunction;
		}
	}

	~FFILLogScope() {
		GetCurrentLog() = PreviousLog;

		if (PreviousWhereFunction.IsSet()) {
			GetCurrentWhereFunction() = PreviousWhereFunction.GetValue();
		}

		if (PreviousStackFunction.IsSet()) {
			GetCurrentStackFunction() = PreviousStackFunction.GetValue();
		}
	}

	FORCEINLINE static UFILLogContainer* GetLog() { return GetCurrentLog(); }
	FORCEINLINE static FString Where() {
		FWhereFunction& Func = GetCurrentWhereFunction();
		if (Func.IsBound()) return Func.Execute();
		return FString();
	}
	FORCEINLINE static FString Stack() {
		FStackFunction& Func = GetCurrentStackFunction();
		if (Func.IsBound()) return Func.Execute();
		return FString();
	}

private:
	static UFILLogContainer*& GetCurrentLog();
	static FWhereFunction& GetCurrentWhereFunction();
	static FStackFunction& GetCurrentStackFunction();

	UFILLogContainer* PreviousLog;
	TOptional<FWhereFunction> PreviousWhereFunction;
	TOptional<FStackFunction> PreviousStackFunction;
};
