#pragma once

#include "CoreMinimal.h"
#include "FIVSMathLib.generated.h"

UCLASS()
class UFIVSMathLib : public UObject {
	GENERATED_BODY()
public:
	UFUNCTION()
	static float FIVSFunc_Float_Addition(float X, float Y) {
		return X + Y;
	}

	UFUNCTION()
	static float FIVSFunc_Float_Subtraction(float X, float Y) {
		return X - Y;
	}
	
	UFUNCTION()
	static float FIVSFunc_Float_Multiply(float X, float Y) {
		return X * Y;
	}

	UFUNCTION()
	static float FIVSFunc_Float_Divide(float X, float Y) {
		return X / Y;
	}

	UFUNCTION()
	static bool FIVSFunc_Float_EqualTo(float X, float Y) {
		return X == Y;
	}

	UFUNCTION()
	static bool FIVSFunc_Float_UnequalTo(float X, float Y) {
		return X != Y;
	}

	UFUNCTION()
	static bool FIVSFunc_Float_LargerThan(float X, float Y) {
		return X > Y;
	}

	UFUNCTION()
	static bool FIVSFunc_Float_LargerOrEqual(float X, float Y) {
		return X >= Y;
	}

	UFUNCTION()
	static bool FIVSFunc_Float_SmallerThan(float X, float Y) {
		return X < Y;
	}

	UFUNCTION()
	bool FIVSFunc_Float_SmallerOrEqual(float X, float Y) {
		return X <= Y;
	}

	UFUNCTION()
	static float FIVSFunc_Float_Sqrt(float X) {
		return FMath::Sqrt(X);
	}

	UFUNCTION()
	static float FIVSFunc_Float_PowerOf(float X, float Y) {
		return FMath::Pow(X, Y);
	}

	UFUNCTION()
	static float FIVSFunc_Float_Sin(float X) {
		return FMath::Sin(X);
	}

	UFUNCTION()
	static float FIVSFunc_Float_Cos(float X) {
		return FMath::Cos(X);
	}

	UFUNCTION()
	static float FIVSFunc_Float_Tan(float X) {
		return FMath::Tan(X);
	}

	UFUNCTION()
	static float FIVSFunc_Float_ASin(float X) {
		return FMath::Asin(X);
	}

	UFUNCTION()
	static float FIVSFunc_Float_ACos(float X) {
		return FMath::Acos(X);
	}

	UFUNCTION()
	static float FIVSFunc_Float_ATan(float X) {
		return FMath::Atan(X);
	}

	UFUNCTION()
	static float FIVSFunc_Float_Abs(float X) {
		return FMath::Abs(X);
	}
};
