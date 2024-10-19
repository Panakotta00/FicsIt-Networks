#include "Reflection/FIRFunction.h"
#include "tracy/Tracy.hpp"

TArray<FFIRAnyValue> UFIRFunction::Execute(const FFIRExecutionContext& Ctx, const TArray<FFIRAnyValue>& Params) const {
	ZoneScopedN("FIRFunction Execute");
	if (NativeFunction) return NativeFunction(Ctx, Params);
	return TArray<FFIRAnyValue>();
}

FString UFIRFunction::ToOperatorName(EFIROperator Operator) {
	switch (Operator) {
		case FIR_Op_Add:
			return TEXT("FIR_Operator_Add");
		case FIR_Op_Sub:
			return TEXT("FIR_Operator_Sub");
		case FIR_Op_Mul:
			return TEXT("FIR_Operator_Mul");
		case FIR_Op_Div:
			return TEXT("FIR_Operator_Div");
		case FIR_Op_Mod:
			return TEXT("FIR_Operator_Mod");
		case FIR_Op_Pow:
			return TEXT("FIR_Operator_Pow");
		case FIR_Op_Neg:
			return TEXT("FIR_Operator_Neg");
		case FIR_Op_FDiv:
			return TEXT("FIR_Operator_FDiv");
		case FIR_Op_BitAND:
			return TEXT("FIR_Operator_BitAND");
		case FIR_Op_BitOR:
			return TEXT("FIR_Operator_BitOR");
		case FIR_Op_BitXOR:
			return TEXT("FIR_Operator_BitXOR");
		case FIR_Op_BitNOT:
			return TEXT("FIR_Operator_BitNOT");
		case FIR_Op_ShiftL:
			return TEXT("FIR_Operator_ShiftL");
		case FIR_Op_ShiftR:
			return TEXT("FIR_Operator_ShiftR");
		case FIR_Op_Concat:
			return TEXT("FIR_Operator_Concat");
		case FIR_Op_Len:
			return TEXT("FIR_Operator_Len");
		case FIR_Op_Equals:
			return TEXT("FIR_Operator_Equals");
		case FIR_Op_LessThan:
			return TEXT("FIR_Operator_LessThan");
		case FIR_Op_LessOrEqualThan:
			return TEXT("FIR_Operator_LessOrEqualThan");
		case FIR_Op_Index:
			return TEXT("FIR_Operator_Index");
		case FIR_Op_NewIndex:
			return TEXT("FIR_Operator_NewIndex");
		case FIR_Op_Call:
			return TEXT("FIR_Operator_Call");
		default:
			return TEXT("");
	}
}

TOptional<EFIROperator> UFIRFunction::ParseOperatorName(FStringView Name) {
	int32 index;
	if (Name.FindLastChar('_', index)) {
		if (Name.Left(index).StartsWith(TEXT("FIR_Operator_"))) {
			Name.LeftInline(index);
		}
	}
	static TMap<FString, EFIROperator> opMap = {
		{TEXT("FIR_Operator_Add"), FIR_Op_Add},
		{TEXT("FIR_Operator_Sub"), FIR_Op_Sub},
		{TEXT("FIR_Operator_Mul"), FIR_Op_Mul},
		{TEXT("FIR_Operator_Div"), FIR_Op_Div},
		{TEXT("FIR_Operator_Mod"), FIR_Op_Mod},
		{TEXT("FIR_Operator_Pow"), FIR_Op_Pow},
		{TEXT("FIR_Operator_Neg"), FIR_Op_Neg},
		{TEXT("FIR_Operator_FDiv"), FIR_Op_FDiv},
		{TEXT("FIR_Operator_BitAND"), FIR_Op_BitAND},
		{TEXT("FIR_Operator_BitOR"), FIR_Op_BitOR},
		{TEXT("FIR_Operator_BitXOR"), FIR_Op_BitXOR},
		{TEXT("FIR_Operator_BitNOT"), FIR_Op_BitNOT},
		{TEXT("FIR_Operator_ShiftL"), FIR_Op_ShiftL},
		{TEXT("FIR_Operator_ShiftR"), FIR_Op_ShiftR},
		{TEXT("FIR_Operator_Concat"), FIR_Op_Concat},
		{TEXT("FIR_Operator_Len"), FIR_Op_Len},
		{TEXT("FIR_Operator_Equals"), FIR_Op_Equals},
		{TEXT("FIR_Operator_LessThan"), FIR_Op_LessThan},
		{TEXT("FIR_Operator_LessOrEqualThan"), FIR_Op_LessOrEqualThan},
		{TEXT("FIR_Operator_Index"), FIR_Op_Index},
		{TEXT("FIR_Operator_NewIndex"), FIR_Op_NewIndex},
		{TEXT("FIR_Operator_Call"), FIR_Op_Call},
	};

	EFIROperator* op = opMap.Find(FString(Name));
	if (op) {
		return *op;
	}
	return {};
}
