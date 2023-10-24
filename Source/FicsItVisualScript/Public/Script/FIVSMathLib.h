#pragma once

#include "CoreMinimal.h"
#include "FIVSNode_UFunctionCall.h"
#include "FIVSMathLib.generated.h"

UCLASS()
class UFIVSMathLib : public UObject {
	GENERATED_BODY()
public:
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Addition, "Addition (float)", "+", "Adds two values", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Addition(float X, float Y) {
		return X + Y;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Subtraction, "Subtraction (float)", "-", "Subtracts two values", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Subtraction(float X, float Y) {
		return X - Y;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Multiply, "Multiply (float)", "*", "Multiplies two values", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Multiply(float X, float Y) {
		return X * Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Divide, "Divide (float)", "/", "Divides two values", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Divide(float X, float Y) {
		return X / Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_EqualTo, "Equals (float)", "==", "Checks if two value are the same", "Math|float");
	UFUNCTION()
	static bool FIVSFunc_Float_EqualTo(float X, float Y) {
		return X == Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_UnequalTo, "Unequal (float)", "!=", "Checks if two value are not the same", "Math|float");
	UFUNCTION()
	static bool FIVSFunc_Float_UnequalTo(float X, float Y) {
		return X != Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_LargerThan, "Larger Than (float)", ">", "Checks if the first value is larger than the second value", "Math|float");
	UFUNCTION()
	static bool FIVSFunc_Float_LargerThan(float X, float Y) {
		return X > Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_LargerOrEqual, "Larger Than or Equal To (float)", ">=", "Checks if the first value is larger than or qual to the second value", "Math|float");
	UFUNCTION()
	static bool FIVSFunc_Float_LargerOrEqual(float X, float Y) {
		return X >= Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_SmallerThan, "Smaller Than (float)", "<", "Checks if the first value is smaller than the second value", "Math|float");
	UFUNCTION()
	static bool FIVSFunc_Float_SmallerThan(float X, float Y) {
		return X < Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_SmallerOrEqual, "Smaller Than or Equal To (float)", "<=", "Checks if the first value is smaller than or eaqual to the second value", "Math|float");
	UFUNCTION()
	bool FIVSFunc_Float_SmallerOrEqual(float X, float Y) {
		return X <= Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Sqrt, "Square Root (float)", "sqrt", "Calculates the square root of the given value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Sqrt(float X) {
		return FMath::Sqrt(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_PowerOf, "Power Of (float)", "pow", "Calculates the first value to the power of the second value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_PowerOf(float X, float Y) {
		return FMath::Pow(X, Y);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Sin, "Sine (float)", "sin", "Calculates the sine of the value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Sin(float X) {
		return FMath::Sin(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Cos, "Cosine (float)", "cos", "Calculates the cosine of the value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Cos(float X) {
		return FMath::Cos(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Tan, "Tangent (float)", "tan", "Calculates the tangent of the value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Tan(float X) {
		return FMath::Tan(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_ASin, "Arcsine (float)", "asin", "Calculates the arcsine of the value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_ASin(float X) {
		return FMath::Asin(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_ACos, "Arccosine (float)", "acos", "Calculates the arccosine of the value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_ACos(float X) {
		return FMath::Acos(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_ATan, "Arctangent (float)", "atan", "Calculates the arctangent of the value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_ATan(float X) {
		return FMath::Atan(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Abs, "Absolute (float)", "abs", "Calculates the absolute of the value.", "Math|float");
	UFUNCTION()
	static float FIVSFunc_Float_Abs(float X) {
		return FMath::Abs(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_Addition, "Addition (int)", "+", "Adds two values", "Math|int");
	UFUNCTION()
	static int64 FIVSFunc_Int_Addition(int64 X, int64 Y) {
		return X + Y;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_Subtraction, "Subtraction (int)", "-", "Subtracts two values", "Math|int");
	UFUNCTION()
	static int64 FIVSFunc_Int_Subtraction(int64 X, int64 Y) {
		return X - Y;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_Multiply, "Multiply (int)", "*", "Multiplies two values", "Math|int");
	UFUNCTION()
	static int64 FIVSFunc_Int_Multiply(int64 X, int64 Y) {
		return X * Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_Divide, "Divide (int)", "/", "Divides two values", "Math|int");
	UFUNCTION()
	static int64 FIVSFunc_Int_Divide(int64 X, int64 Y) {
		return X / Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_Modulo, "Modulo (int)", "%", "Divides two values with the modulo operator, return the left-over.", "Math|int");
	UFUNCTION()
	static int64 FIVSFunc_Int_Modulo(int64 X, int64 Y) {
		return X % Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_EqualTo, "Equals (int)", "==", "Checks if two value are the same", "Math|int");
	UFUNCTION()
	static bool FIVSFunc_Int_EqualTo(int64 X, int64 Y) {
		return X == Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_UnequalTo, "Unequal (int)", "!=", "Checks if two value are not the same", "Math|int");
	UFUNCTION()
	static bool FIVSFunc_Int_UnequalTo(int64 X, int64 Y) {
		return X != Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_LargerThan, "Larger Than (int)", ">", "Checks if the first value is larger than the second value", "Math|int");
	UFUNCTION()
	static bool FIVSFunc_Int_LargerThan(int64 X, int64 Y) {
		return X > Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_LargerOrEqual, "Larger Than or Equal To (int)", ">=", "Checks if the first value is larger than or qual to the second value", "Math|int");
	UFUNCTION()
	static bool FIVSFunc_Int_LargerOrEqual(int64 X, int64 Y) {
		return X >= Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_SmallerThan, "Smaller Than (int)", "<", "Checks if the first value is smaller than the second value", "Math|int");
	UFUNCTION()
	static bool FIVSFunc_Int_SmallerThan(int64 X, int64 Y) {
		return X < Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_SmallerOrEqual, "Smaller Than or Equal To (int)", "<=", "Checks if the first value is smaller than or eaqual to the second value", "Math|int");
	UFUNCTION()
	static bool FIVSFunc_Int_SmallerOrEqual(int64 X, int64 Y) {
		return X <= Y;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Int_Abs, "Absolute (int)", "abs", "Calculates the absolute of the value.", "Math|int");
	UFUNCTION()
	static int64 FIVSFunc_Int_Abs(int64 X) {
		return FMath::Abs(X);
	}
};
