#include "FIVSNode_MathOperation.h"

enum EFIVSMathOperation {
	FIVS_Math_Additon,
	FIVS_Math_Subtraction,
	FIVS_Math_Multiplikation,
	FIVS_Math_Division,
	FIVS_Math_Modulo,
};

template<typename TLeft, typename TRight>
FFINAnyNetworkValue DoOperation(TLeft InLeft, TRight InRight, EFIVSMathOperation Operation) {
	float f1, f2;
	f1 = f1 % f2;
	switch (Operation) {
	case FIVS_Math_Additon:
		return InLeft + InRight;
	case FIVS_Math_Subtraction:
		return InLeft - InRight;
	case FIVS_Math_Multiplikation:
		return InLeft * InRight;
	case FIVS_Math_Division:
		return InLeft / InRight;
	case FIVS_Math_Modulo:
		return InLeft % InRight;
	default:
		return FFINAnyNetworkValue();
	}
}

TArray<FFIVSNodeAction> UFIVSNode_MathOperation::GetNodeActions() const {
	TArray<FFIVSNodeAction> Actions;
	for (EFINNetworkValueType ConvertFromType : TEnumRange<EFINNetworkValueType>()) {
		// Input FIN_ANY is excluded from conversion because it may fail or not and needs its own node
		if (ConvertFromType == FIN_ARRAY || ConvertFromType == FIN_NIL || ConvertFromType == FIN_ANY || ConvertFromType == FIN_STRUCT) continue;
		for (EFINNetworkValueType ConvertToType : TEnumRange<EFINNetworkValueType>()) {
			// Output FIN_ANY is excluded from conversion because it can be casted implicitly and expanded network type allows everything to implicitly convert to any
			if (ConvertToType == FIN_ARRAY || ConvertToType == FIN_NIL || ConvertToType == FIN_STRUCT || ConvertToType == FIN_ANY) continue;

			
		}
	}
	return Actions;
}

void UFIVSNode_MathOperation::InitPins() {
	
}

FString UFIVSNode_MathOperation::GetNodeName() const {
	return Super::GetNodeName();
}

TArray<UFIVSPin*> UFIVSNode_MathOperation::PreExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return Super::PreExecPin(ExecPin, Context);
}

UFIVSPin* UFIVSNode_MathOperation::ExecPin(UFIVSPin* ExecPin, FFIVSRuntimeContext& Context) {
	return Super::ExecPin(ExecPin, Context);
}
