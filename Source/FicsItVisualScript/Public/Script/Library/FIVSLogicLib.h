#pragma once

#include "CoreMinimal.h"
#include "FIVSCompileLua.h"
#include "FIVSNode_LuaGeneric.h"
#include "FIVSLogicLib.generated.h"

UCLASS()
class UFIVSLogicLib : public UObject {
	GENERATED_BODY()
public:
	FIVSNode_BeginLuaGenericMeta(FIVSFunc_And, "And", "&&", "Combines two boolean values with a AND connection.", "Logic")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("And" , "And", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_And(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* And) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(And, FString::Printf(TEXT("(%s and %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Or, "Or", "||", "Combines two boolean values with a OR connection.", "Logic")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Or" , "Or", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Or(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Or) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Or, FString::Printf(TEXT("(%s or %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Not, "Not", "!", "Negates a boolean value.", "Logic")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Neg" , "Neg", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Not(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Neg) {
		FString OP1 = Context.GetRValueExpression(X);
		Context.AddExpression(Neg, FString::Printf(TEXT("(not %s)"), *OP1));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_EqualTo, "Equal To (Boolean)", "==", "Checks if two boolean values are the same.", "Logic")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Eq" , "Eq", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_EqualTo(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Eq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Eq, FString::Printf(TEXT("(%s == %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_UnequalTo, "Unequal To (Boolean)", "!=", "Checks if two boolean values are the not same.", "Logic")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("NEq" , "NEq", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_UnequalTo(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* NEq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(NEq, FString::Printf(TEXT("(%s ~= %s)"), *OP1, *OP2));
	}
};
