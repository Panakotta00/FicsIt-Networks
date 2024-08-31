#include "Reflection/FINFunction.h"
#include "tracy/Tracy.hpp"

TArray<FFINAnyNetworkValue> UFINFunction::Execute(const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params) const {
	ZoneScopedN("FINFunction Execute");
	if (NativeFunction) return NativeFunction(Ctx, Params);
	return TArray<FFINAnyNetworkValue>();
}

FString UFINFunction::ToOperatorName(EFINOperator Operator) {
	switch (Operator) {
		case FIN_Op_Add:
			return TEXT("FIN_Operator_Add");
		case FIN_Op_Sub:
			return TEXT("FIN_Operator_Sub");
		case FIN_Op_Mul:
			return TEXT("FIN_Operator_Mul");
		case FIN_Op_Div:
			return TEXT("FIN_Operator_Div");
		case FIN_Op_Mod:
			return TEXT("FIN_Operator_Mod");
		case FIN_Op_Pow:
			return TEXT("FIN_Operator_Pow");
		case FIN_Op_Neg:
			return TEXT("FIN_Operator_Neg");
		case FIN_Op_FDiv:
			return TEXT("FIN_Operator_FDiv");
		case FIN_Op_BitAND:
			return TEXT("FIN_Operator_BitAND");
		case FIN_Op_BitOR:
			return TEXT("FIN_Operator_BitOR");
		case FIN_Op_BitXOR:
			return TEXT("FIN_Operator_BitXOR");
		case FIN_Op_BitNOT:
			return TEXT("FIN_Operator_BitNOT");
		case FIN_Op_ShiftL:
			return TEXT("FIN_Operator_ShiftL");
		case FIN_Op_ShiftR:
			return TEXT("FIN_Operator_ShiftR");
		case FIN_Op_Concat:
			return TEXT("FIN_Operator_Concat");
		case FIN_Op_Len:
			return TEXT("FIN_Operator_Len");
		case FIN_Op_Equals:
			return TEXT("FIN_Operator_Equals");
		case FIN_Op_LessThan:
			return TEXT("FIN_Operator_LessThan");
		case FIN_Op_LessOrEqualThan:
			return TEXT("FIN_Operator_LessOrEqualThan");
		case FIN_Op_Index:
			return TEXT("FIN_Operator_Index");
		case FIN_Op_NewIndex:
			return TEXT("FIN_Operator_NewIndex");
		case FIN_Op_Call:
			return TEXT("FIN_Operator_Call");
		default:
			return TEXT("");
	}
}

TOptional<EFINOperator> UFINFunction::ParseOperatorName(FStringView Name) {
	int32 index;
	if (Name.FindLastChar('_', index)) {
		if (Name.Left(index).StartsWith(TEXT("FIN_Operator_"))) {
			Name.LeftInline(index);
		}
	}
	static TMap<FString, EFINOperator> opMap = {
		{TEXT("FIN_Operator_Add"), FIN_Op_Add},
		{TEXT("FIN_Operator_Sub"), FIN_Op_Sub},
		{TEXT("FIN_Operator_Mul"), FIN_Op_Mul},
		{TEXT("FIN_Operator_Div"), FIN_Op_Div},
		{TEXT("FIN_Operator_Mod"), FIN_Op_Mod},
		{TEXT("FIN_Operator_Pow"), FIN_Op_Pow},
		{TEXT("FIN_Operator_Neg"), FIN_Op_Neg},
		{TEXT("FIN_Operator_FDiv"), FIN_Op_FDiv},
		{TEXT("FIN_Operator_BitAND"), FIN_Op_BitAND},
		{TEXT("FIN_Operator_BitOR"), FIN_Op_BitOR},
		{TEXT("FIN_Operator_BitXOR"), FIN_Op_BitXOR},
		{TEXT("FIN_Operator_BitNOT"), FIN_Op_BitNOT},
		{TEXT("FIN_Operator_ShiftL"), FIN_Op_ShiftL},
		{TEXT("FIN_Operator_ShiftR"), FIN_Op_ShiftR},
		{TEXT("FIN_Operator_Concat"), FIN_Op_Concat},
		{TEXT("FIN_Operator_Len"), FIN_Op_Len},
		{TEXT("FIN_Operator_Equals"), FIN_Op_Equals},
		{TEXT("FIN_Operator_LessThan"), FIN_Op_LessThan},
		{TEXT("FIN_Operator_LessOrEqualThan"), FIN_Op_LessOrEqualThan},
		{TEXT("FIN_Operator_Index"), FIN_Op_Index},
		{TEXT("FIN_Operator_NewIndex"), FIN_Op_NewIndex},
		{TEXT("FIN_Operator_Call"), FIN_Op_Call},
	};

	EFINOperator* op = opMap.Find(FString(Name));
	if (op) {
		return *op;
	}
	return {};
}
