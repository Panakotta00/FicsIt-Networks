#pragma once

#include "CoreMinimal.h"
#include "FIVSNode_UFunctionCall.h"
#include "FIVSLogicLib.generated.h"

UCLASS()
class UFIVSLogicLib : public UObject {
	GENERATED_BODY()
public:
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_And, "And", "&&", "Combines two boolean values with a AND connection.", "Logic");
	UFUNCTION()
	static bool FIVSFunc_And(bool A, bool B) {
		return A && B;
	}
	
	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Or, "Or", "||", "Combines two boolean values with a OR connection.", "Logic");
	UFUNCTION()
	static bool FIVSFunc_Or(bool A, bool B) {
		return A || B;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_Not, "Not", "!", "Negates a boolean value.", "Logic");
	UFUNCTION()
	static bool FIVSFunc_Not(bool A, bool B) {
		return A && B;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_EqualTo, "Equal To", "==", "Checks if two boolean values are the same.", "Logic");
	UFUNCTION()
	static bool FIVSFunc_EqualTo(bool A, bool B) {
		return A == B;
	}

	FIVSNode_UFunctionOperatorMeta(FIVSFunc_UnequalTo, "Unequal To", "!=", "Checks if two boolean values are not the same.", "Logic");
	UFUNCTION()
	static bool FIVSFunc_UnequalTo(bool A, bool B) {
		return A != B;
	}
};