#pragma once

#include "CoreMinimal.h"
#include "FIVSNode_LuaGeneric.h"
#include "Script/FIVSCompileLua.h"
#include "FIVSMathLib.generated.h"

UCLASS()
class UFIVSMathLib : public UObject {
	GENERATED_BODY()
public:
	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Addition, "Lua Addition (float)", "+", "Adds two Floats", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Sum" , "Sum", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Addition(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Sum) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Sum, FString::Printf(TEXT("(%s + %s)"), *OP1, *OP2));
	}
	
	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Subtraction, "Subtraction (float)", "-", "Subtracts two Floats", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Sub" , "Sub", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Subtraction(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Sub) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Sub, FString::Printf(TEXT("(%s - %s)"), *OP1, *OP2));
	}
	
	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Multiply, "Multiplication (float)", "*", "Multiplies two Floats", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Mul" , "Mul", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Multiply(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Mul) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Mul, FString::Printf(TEXT("(%s * %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Divide, "Division (float)", "/", "Divides two Floats", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Div" , "Div", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Divide(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Div) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Div, FString::Printf(TEXT("(%s / %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_EqualTo, "Equals (float)", "==", "Checks if two value are the same", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Eq" , "Eq", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_EqualTo(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Eq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Eq, FString::Printf(TEXT("(%s == %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_UnequalTo, "Unequal (float)", "!=", "Checks if two value are not the same", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("NEq" , "NEq", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_UnequalTo(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* NEq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(NEq, FString::Printf(TEXT("(%s ~= %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_LargerThan, "Greater Than (float)", ">", "Checks if the first value is greater than the second value", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("GT" , "GT", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_LargerThan(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* GT) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(GT, FString::Printf(TEXT("(%s > %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_LargerOrEqual, "Greater Than or Equal To (float)", ">=", "Checks if the first value is greater than or equal to the second value", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("GTEq" , "GTEq", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_LargerOrEqual(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* GTEq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(GTEq, FString::Printf(TEXT("(%s >= %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_SmallerThan, "Less Than (float)", ">=", "Checks if the first value is less than the second value", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("LT" , "LT", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_SmallerThan(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* LT) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(LT, FString::Printf(TEXT("(%s < %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_SmallerOrEqual, "Less Than or Equal To (float)", "<=", "Checks if the first value is less than or equal to the second value", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("LTEq" , "LTEq", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_SmallerOrEqual(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* LTEq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(LTEq, FString::Printf(TEXT("(%s <= %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Sqrt, "Square Root (float)", "sqrt", "Calculates the square root of the given value.", "Math|float")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Sqrt" , "Sqrt", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Sqrt(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Sqrt) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(Sqrt, FString::Printf(TEXT("math.sqrt(%s)"), *Val));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_PowerOf, "Power Of (float)", "pow", "Calculates the first value to the power of the second value.", "Math|float");
	FIVSNode_LuaGenericPin("Base" , "Base", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("Exp" , "Exp", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("pow", "pow", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_PowerOf(float X, UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* Base, UFIVSPin* Exp, UFIVSPin* pow) {
		FString valBase = Context.GetRValueExpression(Base);
		FString valExp = Context.GetRValueExpression(Exp);
		Context.AddExpression(pow, FString::Printf(TEXT("math.pow(%s, %s)"), *valBase, *valExp));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Sin, "Sine (float)", "sin", "Calculates the sine of the value.", "Math|float");
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("sin", "sin", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Sin(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* sin) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(sin, FString::Printf(TEXT("math.sin(%s)"), *Val));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Cos, "Cosine (float)", "cos", "Calculates the cosine of the value.", "Math|float");
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("cos", "cos", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Cos(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* cos) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(cos, FString::Printf(TEXT("math.cos(%s)"), *Val));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Tan, "Tangent (float)", "tan", "Calculates the tangent of the value.", "Math|float");
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("tan", "tan", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Tan(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* tan) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(tan, FString::Printf(TEXT("math.tan(%s)"), *Val));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_ASin, "Arcsine (float)", "asin", "Calculates the arcsine of the value.", "Math|float");
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("asin", "asin", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_ASin(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* asin) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(asin, FString::Printf(TEXT("math.asin(%s)"), *Val));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_ACos, "Arccosine (float)", "acos", "Calculates the arccosine of the value.", "Math|float");
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("acos", "acos", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_ACos(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* acos) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(acos, FString::Printf(TEXT("math.acos(%s)"), *Val));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_ATan, "Arctangent (float)", "atan", "Calculates the arctangent of the value.", "Math|float");
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("atan", "atan", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_ATan(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* atan) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(atan, FString::Printf(TEXT("math.atan(%s)"), *Val));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Abs, "Absolute (float)", "abs", "Calculates the absolute of the value.", "Math|float");
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_FLOAT)
	FIVSNode_LuaGenericPin("abs", "abs", FIVS_PIN_DATA_OUTPUT, FIR_FLOAT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Float_Abs(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* abs) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(abs, FString::Printf(TEXT("math.abs(%s)"), *Val));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Addition, "Lua Addition (int)", "+", "Adds two Floats", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Sum" , "Sum", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_Addition(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Sum) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Sum, FString::Printf(TEXT("(%s + %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Subtraction, "Subtraction (int)", "-", "Subtracts two Floats", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Sub" , "Sub", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_Subtraction(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Sub) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Sub, FString::Printf(TEXT("(%s - %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Multiply, "Multiplication (int)", "*", "Multiplies two Floats", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Mul" , "Mul", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_Multiply(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Mul) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Mul, FString::Printf(TEXT("(%s * %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_Divide, "Division (int)", "/", "Divides two Floats", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Div" , "Div", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_Divide(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Div) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Div, FString::Printf(TEXT("(%s / %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_EqualTo, "Equals (int)", "==", "Checks if two value are the same", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Eq" , "Eq", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_EqualTo(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* Eq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(Eq, FString::Printf(TEXT("(%s == %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_UnequalTo, "Unequal (int)", "!=", "Checks if two value are not the same", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("NEq" , "NEq", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_UnequalTo(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* NEq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(NEq, FString::Printf(TEXT("(%s ~= %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_LargerThan, "Greater Than (int)", ">", "Checks if the first value is greater than the second value", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("GT" , "GT", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_LargerThan(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* GT) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(GT, FString::Printf(TEXT("(%s > %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_LargerOrEqual, "Greater Than or Equal To (int)", ">=", "Checks if the first value is greater than or equal to the second value", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("GTEq" , "GTEq", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_LargerOrEqual(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* GTEq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(GTEq, FString::Printf(TEXT("(%s >= %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_SmallerThan, "Less Than (int)", ">=", "Checks if the first value is less than the second value", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("LT" , "LT", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_SmallerThan(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* LT) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(LT, FString::Printf(TEXT("(%s < %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Float_SmallerOrEqual, "Less Than or Equal To (int)", "<=", "Checks if the first value is less than or equal to the second value", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Y" , "Y", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("LTEq" , "LTEq", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_SmallerOrEqual(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Y, UFIVSPin* LTEq) {
		FString OP1 = Context.GetRValueExpression(X);
		FString OP2 = Context.GetRValueExpression(Y);
		Context.AddExpression(LTEq, FString::Printf(TEXT("(%s <= %s)"), *OP1, *OP2));
	}

	FIVSNode_BeginLuaGenericMeta(FIVSFunc_Int_Abs, "Absolute (int)", "abs", "Calculates the absolute of the value.", "Math|int")
	FIVSNode_LuaGenericPin("X" , "X", FIVS_PIN_DATA_INPUT, FIR_INT)
	FIVSNode_LuaGenericPin("Abs" , "Abs", FIVS_PIN_DATA_OUTPUT, FIR_INT)
	FIVSNode_EndLuaGenericMeta()
	UFUNCTION()
	static void FIVSFunc_Int_Abs(UPARAM(ref) FFIVSLuaCompilerContext& Context, UFIVSPin* X, UFIVSPin* Abs) {
		FString Val = Context.GetRValueExpression(X);
		Context.AddExpression(Abs, FString::Printf(TEXT("math.abs(%s)"), *Val));
	}
};
