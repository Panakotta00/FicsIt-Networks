#pragma once

#include "CoreMinimal.h"
#include "FIVSNode_LuaGeneric.h"
#include "FIVSStringLib.generated.h"

UCLASS()
class UFIVSStringLib : public UObject {
	GENERATED_BODY()
public:
	FIVSNode_BeginLuaGenericMeta(FIVSFunc_String_Append, "Append String", "..", "Combines two strings A and B together into one", "Math|string")
	FIVSNode_LuaGenericPin("A" , "A", FIVS_PIN_DATA_INPUT, FIR_STR)
	FIVSNode_LuaGenericPin("B" , "B", FIVS_PIN_DATA_INPUT, FIR_STR)
	FIVSNode_LuaGenericPin("Out" , "Out", FIVS_PIN_DATA_OUTPUT, FIR_STR)
	FIVSNode_EndLuaGenericMeta();
	UFUNCTION()
	static void FIVSFunc_String_Append(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* A, UFIVSPin* B, UFIVSPin* Out) {
		FString OP1 = Context.GetRValueExpression(A);
		FString OP2 = Context.GetRValueExpression(B);
		Context.AddExpression(Out, FString::Printf(TEXT("(%s .. %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_String_FromInt, "To String (int)", "·", "Converts an integer into a string", "Math|string")
	FIVSNode_LuaGenericPin("Val" , "Val", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Out" , "Out", FIVS_PIN_DATA_OUTPUT, FIR_STR)
	FIVSNode_EndLuaGenericMeta();
	UFUNCTION()
	static void FIVSFunc_String_FromInt(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* Val, UFIVSPin* Out) {
		FString OP1 = Context.GetRValueExpression(Val);
		Context.AddExpression(Out, FString::Printf(TEXT("tostring(%s)"), *OP1));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_String_FromFloat, "To String (float)", "·", "Converts a float into a string", "Math|string")
	FIVSNode_LuaGenericPin("Val" , "Val", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Out" , "Out", FIVS_PIN_DATA_OUTPUT, FIR_STR)
	FIVSNode_EndLuaGenericMeta();
	UFUNCTION()
	static void FIVSFunc_String_FromFloat(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* Val, UFIVSPin* Out) {
		FString OP1 = Context.GetRValueExpression(Val);
		Context.AddExpression(Out, FString::Printf(TEXT("tostring(%s)"), *OP1));
	}
};
