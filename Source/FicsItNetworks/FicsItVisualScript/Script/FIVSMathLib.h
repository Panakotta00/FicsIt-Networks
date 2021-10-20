#pragma once

#include "CoreMinimal.h"
#include "FIVSNode_UFunctionCall.h"
#include "FIVSMathLib.generated.h"

UCLASS()
class UFIVSMathLib : public UObject {
	GENERATED_BODY()
public:
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Addition, "Add", "+", "Adds two values", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Addition(float X, float Y) {
		return X + Y;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Subtraction, "Subtraction", "-", "Subtracts two values", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Subtraction(float X, float Y) {
		return X - Y;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Multiply, "Multiply", "*", "Multiplies two values", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Multiply(float X, float Y) {
		return X * Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Divide, "Divide", "/", "Divides two values", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Divide(float X, float Y) {
		return X / Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_EqualTo, "Equals", "==", "Checks if two value are the same", "Math");
	UFUNCTION()
	static bool FIVSFunc_Float_EqualTo(float X, float Y) {
		return X == Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_UnequalTo, "Unequal", "!=", "Checks if two value are not the same", "Math");
	UFUNCTION()
	static bool FIVSFunc_Float_UnequalTo(float X, float Y) {
		return X != Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_LargerThan, "Larger Than", ">", "Checks if the first value is larger than the second value", "Math");
	UFUNCTION()
	static bool FIVSFunc_Float_LargerThan(float X, float Y) {
		return X > Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_LargerOrEqual, "Larger Than or Equal To", ">=", "Checks if the first value is larger than or qual to the second value", "Math");
	UFUNCTION()
	static bool FIVSFunc_Float_LargerOrEqual(float X, float Y) {
		return X >= Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_SmallerThan, "Smaller Than", "<", "Checks if the first value is smaller than the second value", "Math");
	UFUNCTION()
	static bool FIVSFunc_Float_SmallerThan(float X, float Y) {
		return X < Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_SmallerOrEqual, "Smaller Than or Equal To", "<=", "Checks if the first value is smaller than or eaqual to the second value", "Math");
	UFUNCTION()
	bool FIVSFunc_Float_SmallerOrEqual(float X, float Y) {
		return X <= Y;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Sqrt, "Square Root", "sqrt", "Calculates the square root of the given value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Sqrt(float X) {
		return FMath::Sqrt(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_PowerOf, "Power Of", "pow", "Calculates the first value to the power of the second value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_PowerOf(float X, float Y) {
		return FMath::Pow(X, Y);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Sin, "Sine", "sin", "Calculates the sine of the value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Sin(float X) {
		return FMath::Sin(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Cos, "Cosine", "cos", "Calculates the cosine of the value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Cos(float X) {
		return FMath::Cos(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Tan, "Tangent", "tan", "Calculates the tangent of the value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Tan(float X) {
		return FMath::Tan(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_ASin, "Arcsine", "asin", "Calculates the arcsine of the value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_ASin(float X) {
		return FMath::Asin(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_ACos, "Arccosine", "acos", "Calculates the arccosine of the value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_ACos(float X) {
		return FMath::Acos(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_ATan, "Arctangent", "atan", "Calculates the arctangent of the value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_ATan(float X) {
		return FMath::Atan(X);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Float_Abs, "Absolute", "abs", "Calculates the absolute of the value.", "Math");
	UFUNCTION()
	static float FIVSFunc_Float_Abs(float X) {
		return FMath::Abs(X);
	}
};
