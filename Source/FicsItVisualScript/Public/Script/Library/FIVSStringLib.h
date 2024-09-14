#pragma once

#include "CoreMinimal.h"
#include "FIVSNode_UFunctionCall.h"
#include "FIVSStringLib.generated.h"
/*
UCLASS()
class UFIVSStringLib : public UObject {
	GENERATED_BODY()
public:
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Append, "Append", "+", "Combinds the given string together.", "String");
	UFUNCTION()
	static FString FIVSFunc_Append(const FString& A, const FString& B) {
		return FString(A).Append(B);
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Substr, "Sub-String", "substr", "Returns a part of the string.", "String");
	UFUNCTION()
	static FString FIVSFunc_Substr(FString A, int StartIndex, int Count) {
		return A.Mid(StartIndex, Count);
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_EqualTo, "Equal To", "==", "Compares the values and returns if true if they are the same.", "String");
	UFUNCTION()
	static bool FIVSFunc_EqualTo(FString A, FString B) {
		return A == B;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_UnequalTo, "Unequal To", "!=", "Compares the values and returns if true if they are not the same.", "String");
	UFUNCTION()
	static bool FIVSFunc_UnequalTo(FString A, FString B) {
		return A != B;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Compare, "Compare", "==", "Compares the values and returns a value that shows how they compare. 0 = The same, <0 B is smaller, >0 B is bigger.", "String");
	UFUNCTION()
	static int FIVSFunc_Compare(FString A, FString B) {
		return A.Compare(B);
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Length, "Length", "Len", "Returns the length of the string.", "String");
	UFUNCTION()
	static int FIVSFunc_Length(FString A) {
		return A.Len();
	}
};
*/