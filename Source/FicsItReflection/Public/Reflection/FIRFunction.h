#pragma once

#include "CoreMinimal.h"
#include "FIRProperty.h"
#include "FIRException.h"
#include "FIRFunction.generated.h"

#define FIR_Operator_Add operatorAdd
#define FIR_Operator_Sub operatorSub
#define FIR_Operator_Mul operatorMul
#define FIR_Operator_Div operatorDiv
#define FIR_Operator_Mod operatorMod
#define FIR_Operator_Pow peratorPow
#define FIR_Operator_Neg operatorNeg
#define FIR_Operator_FDiv operatorFDiv
#define FIR_Operator_BitAND operatorBitAND
#define FIR_Operator_BitOR operatorBitOR
#define FIR_Operator_BitXOR operatorBitXOR
#define FIR_Operator_BitNOT operatorBitNOT
#define FIR_Operator_ShiftL operatorLShift
#define FIR_Operator_ShiftR operatorRShift
#define FIR_Operator_Concat operatorConcat
#define FIR_Operator_Len operatorLen
#define FIR_Operator_Equals operatorEquals
#define FIR_Operator_LessThan operatorLessThan
#define FIR_Operator_LessOrEqualThan operatorLessOrEqualThan
#define FIR_Operator_Index operatorIndex
#define FIR_Operator_NewIndex operatorNewIndex
#define FIR_Operator_Call operatorCall

#define FIR_OP_TEXT(OP) #OP

enum EFIRFunctionFlags {
	FIR_Func_None			= 0b000000000,
	FIR_Func_VarArgs		= 0b000000001,
	FIR_Func_Runtime		= 0b000001110,
	FIR_Func_RT_Sync		= 0b000000010,
	FIR_Func_RT_Parallel	= 0b000000100,
	FIR_Func_RT_Async		= 0b000001000,
	FIR_Func_Sync			= 0b000000010,
	FIR_Func_Parallel		= 0b000000110,
	FIR_Func_Async			= 0b000001110,
	FIR_Func_MemberFunc		= 0b000010000,
	FIR_Func_ClassFunc		= 0b000100000,
	FIR_Func_StaticFunc		= 0b001000000,
	FIR_Func_VarRets		= 0b010000000,
	FIR_Func_StaticSource	= 0b100000000,
};

ENUM_CLASS_FLAGS(EFIRFunctionFlags)

UENUM()
enum EFIROperator {
	// Addition
	FIR_Op_Add,

	// Subtraction
	FIR_Op_Sub,

	// Multiplication
	FIR_Op_Mul,

	// Division
	FIR_Op_Div,

	// Modulo
	FIR_Op_Mod,

	// Power/Exponent
	FIR_Op_Pow,

	// Negation / Unary Minus
	FIR_Op_Neg,

	// Floot Division
	FIR_Op_FDiv,

	// Bitwise AND
	FIR_Op_BitAND,

	// Bitwise OR
	FIR_Op_BitOR,

	// Bitwise XOR
	FIR_Op_BitXOR,

	// Bitwise NOT/Negation
	FIR_Op_BitNOT,

	// Left Shift
	FIR_Op_ShiftL,

	// Right Shift
	FIR_Op_ShiftR,

	// Concat
	FIR_Op_Concat,

	// Length
	FIR_Op_Len,

	// Equal
	FIR_Op_Equals,

	// Less Than
	FIR_Op_LessThan,

	// Less Than Or Equal To
	FIR_Op_LessOrEqualThan,

	// Index `[]`
	FIR_Op_Index,

	// New Index `[] =`
	FIR_Op_NewIndex,

	// Call `()`
	FIR_Op_Call,
};

USTRUCT()
struct FICSITREFLECTION_API FFIRFunctionBadArgumentException : public FFIRReflectionException {
	GENERATED_BODY()

	int ArgumentIndex = 0;

	FFIRFunctionBadArgumentException() = default;
	FFIRFunctionBadArgumentException(UFIRFunction* Func, int ArgumentIndex, const FString& Message) : FFIRReflectionException(Cast<UFIRBase>(Func), Message), ArgumentIndex(ArgumentIndex) {}
};

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFIRFunction : public UFIRBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<UFIRProperty*> Parameters;
	
	TFunction<TArray<FFIRAnyValue>(const FFIRExecutionContext&, const TArray<FFIRAnyValue>&)> NativeFunction;

	EFIRFunctionFlags FunctionFlags = FIR_Func_Sync;
	
	/**
	 * Returns a list of all the parameters this function has
	 */
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
    virtual TArray<UFIRProperty*> GetParameters() const { return Parameters; }

	/**
	 * Returns the function flags of this function
	 */
	virtual EFIRFunctionFlags GetFunctionFlags() const { return FunctionFlags; }

	/**
	 * Executes the function with the given properties and the given Ctx
	 */
	virtual TArray<FFIRAnyValue> Execute(const FFIRExecutionContext& Ctx, const TArray<FFIRAnyValue>& Params) const;

	TOptional<EFIROperator> IsOperator() {
		return ParseOperatorName(InternalName);
	}

	UFUNCTION()
	static FString ToOperatorName(EFIROperator Operator);

	static TOptional<EFIROperator> ParseOperatorName(FStringView Name);
};
