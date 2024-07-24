#pragma once

#include "FINExecutionContext.h"
#include "FINProperty.h"
#include "FINReflectionException.h"
#include "FINFunction.generated.h"

#define FIN_Operator_Add operatorAdd
#define FIN_Operator_Sub operatorSub
#define FIN_Operator_Mul operatorMul
#define FIN_Operator_Div operatorDiv
#define FIN_Operator_Mod operatorMod
#define FIN_Operator_Pow peratorPow
#define FIN_Operator_Neg operatorNeg
#define FIN_Operator_FDiv operatorFDiv
#define FIN_Operator_BitAND operatorBitAND
#define FIN_Operator_BitOR operatorBitOR
#define FIN_Operator_BitXOR operatorBitXOR
#define FIN_Operator_BitNOT operatorBitNOT
#define FIN_Operator_ShiftL operatorLShift
#define FIN_Operator_ShiftR operatorRShift
#define FIN_Operator_Concat operatorConcat
#define FIN_Operator_Len operatorLen
#define FIN_Operator_Equals operatorEquals
#define FIN_Operator_LessThan operatorLessThan
#define FIN_Operator_LessOrEqualThan operatorLessOrEqualThan
#define FIN_Operator_Index operatorIndex
#define FIN_Operator_NewIndex operatorNewIndex
#define FIN_Operator_Call operatorCall

#define FIN_OP_TEXT(OP) #OP

enum EFINFunctionFlags {
	FIN_Func_None			= 0b000000000,
	FIN_Func_VarArgs		= 0b000000001,
	FIN_Func_Runtime		= 0b000001110,
	FIN_Func_RT_Sync		= 0b000000010,
	FIN_Func_RT_Parallel	= 0b000000100,
	FIN_Func_RT_Async		= 0b000001000,
	FIN_Func_Sync			= 0b000000010,
	FIN_Func_Parallel		= 0b000000110,
	FIN_Func_Async			= 0b000001110,
	FIN_Func_MemberFunc		= 0b000010000,
	FIN_Func_ClassFunc		= 0b000100000,
	FIN_Func_StaticFunc		= 0b001000000,
	FIN_Func_VarRets		= 0b010000000,
	FIN_Func_StaticSource	= 0b100000000,
};

ENUM_CLASS_FLAGS(EFINFunctionFlags)

UENUM()
enum EFINOperator {
	// Addition
	FIN_Op_Add,

	// Subtraction
	FIN_Op_Sub,

	// Multiplication
	FIN_Op_Mul,

	// Division
	FIN_Op_Div,

	// Modulo
	FIN_Op_Mod,

	// Power/Exponent
	FIN_Op_Pow,

	// Negation / Unary Minus
	FIN_Op_Neg,

	// Floot Division
	FIN_Op_FDiv,

	// Bitwise AND
	FIN_Op_BitAND,

	// Bitwise OR
	FIN_Op_BitOR,

	// Bitwise XOR
	FIN_Op_BitXOR,

	// Bitwise NOT/Negation
	FIN_Op_BitNOT,

	// Left Shift
	FIN_Op_ShiftL,

	// Right Shift
	FIN_Op_ShiftR,

	// Concat
	FIN_Op_Concat,

	// Length
	FIN_Op_Len,

	// Equal
	FIN_Op_Equals,

	// Less Than
	FIN_Op_LessThan,

	// Less Than Or Equal To
	FIN_Op_LessOrEqualThan,

	// Index `[]`
	FIN_Op_Index,

	// New Index `[] =`
	FIN_Op_NewIndex,

	// Call `()`
	FIN_Op_Call,
};

USTRUCT()
struct FICSITNETWORKS_API FFINFunctionBadArgumentException : public FFINReflectionException {
	GENERATED_BODY()

	int ArgumentIndex = 0;

	FFINFunctionBadArgumentException() = default;
	FFINFunctionBadArgumentException(UFINFunction* Func, int ArgumentIndex, const FString& Message) : FFINReflectionException(Cast<UFINBase>(Func), Message), ArgumentIndex(ArgumentIndex) {}
};

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINFunction : public UFINBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<UFINProperty*> Parameters;
	
	TFunction<TArray<FFINAnyNetworkValue>(const FFINExecutionContext&, const TArray<FFINAnyNetworkValue>&)> NativeFunction;

	EFINFunctionFlags FunctionFlags = FIN_Func_Sync;
	
	/**
	 * Returns a list of all the parameters this function has
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual TArray<UFINProperty*> GetParameters() const { return Parameters; }

	/**
	 * Returns the function flags of this function
	 */
	virtual EFINFunctionFlags GetFunctionFlags() const { return FunctionFlags; }

	/**
	 * Executes the function with the given properties and the given Ctx
	 */
	virtual TArray<FFINAnyNetworkValue> Execute(const FFINExecutionContext& Ctx, const TArray<FFINAnyNetworkValue>& Params) const;

	TOptional<EFINOperator> IsOperator() {
		return ParseOperatorName(InternalName);
	}

	UFUNCTION()
	static FString ToOperatorName(EFINOperator Operator);

	static TOptional<EFINOperator> ParseOperatorName(FStringView Name);
};
