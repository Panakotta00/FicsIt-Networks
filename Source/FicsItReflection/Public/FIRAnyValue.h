#pragma once

#include "CoreMinimal.h"
#include "Misc/DefaultValueHelper.h"
#include "FIRTypes.h"
#include "FIRAnyValue.generated.h"

/**
 * This sturcture allows you to store any kind of network value.
 */
USTRUCT(BlueprintType)
struct FICSITREFLECTION_API FFIRAnyValue {
	GENERATED_BODY()

	FORCEINLINE static FFIRAnyValue DefaultValue(EFIRValueType ValueType) {
		FFIRAnyValue Value;
		Value.Type = ValueType;
		auto& Data = Value.Data;
		switch (ValueType) {
			case FIR_NIL:
				break;
			case FIR_BOOL:
				Data.BOOL = false;
			break;
			case FIR_INT:
				Data.INT = 0;
			break;
			case FIR_FLOAT:
				Data.FLOAT = 0.0f;
			break;
			case FIR_STR:
				Data.STRING = new FIRStr();
			break;
			case FIR_OBJ:
				Data.OBJECT = new FIRObj();
			break;
			case FIR_CLASS:
				Data.CLASS = nullptr;
			break;
			case FIR_TRACE:
				Data.TRACE = new FIRTrace();
			break;
			case FIR_STRUCT:
				Data.STRUCT = new FIRStruct();
			break;
			case FIR_ARRAY:
				Data.ARRAY = new FIRArray();
			break;
			case FIR_ANY:
				Data.ANY = new FIRAny();
			break;
		}
		return Value;
	}
	
	FORCEINLINE FFIRAnyValue() : Type(FIR_NIL), Data() {}

	FORCEINLINE FFIRAnyValue(FIRInt e) {
		Data.INT = e;
		Type = FIR_INT;
	}

	FORCEINLINE FFIRAnyValue(FIRFloat e) {
		Data.FLOAT = e;
		Type = FIR_FLOAT;
	}

	FORCEINLINE FFIRAnyValue(FIRBool e) {
		Data.BOOL = e;
		Type = FIR_BOOL;
	}

	FORCEINLINE FFIRAnyValue(FIRClass e) {
		Data.CLASS = e;
		Type = FIR_CLASS;
	}

	FORCEINLINE FFIRAnyValue(const FIRStr& e) {
		Data.STRING = new FIRStr(e);
		Type = FIR_STR;
	}

	FORCEINLINE FFIRAnyValue(const FIRObj& e) {
		Data.OBJECT = new FIRObj(e);
		Type = FIR_OBJ;
	}

	FORCEINLINE FFIRAnyValue(const FIRTrace& e) {
		Data.TRACE = new FIRTrace(e);
		Type = FIR_TRACE;
	}

	FORCEINLINE FFIRAnyValue(const FIRStruct& e) {
		Data.STRUCT = new FIRStruct(e);
		Type = FIR_STRUCT;
	}

	FORCEINLINE FFIRAnyValue(const FIRArray& e) {
		Data.ARRAY = new FIRArray(e);
		Type = FIR_ARRAY;
	}

	FORCEINLINE FFIRAnyValue(const FFIRAnyValue& other) {
		*this = other;
	}

	FORCEINLINE FFIRAnyValue& operator=(const FFIRAnyValue& other) {
		this->~FFIRAnyValue();
		Type = other.Type;
		switch (Type) {
		case FIR_STR:
			Data.STRING = new FIRStr(*other.Data.STRING);
			break;
		case FIR_OBJ:
			Data.OBJECT = new FIRObj(*other.Data.OBJECT);
			break;
		case FIR_TRACE:
			Data.TRACE = new FIRTrace(*other.Data.TRACE);
			break;
		case FIR_STRUCT:
			Data.STRUCT = new FIRStruct(*other.Data.STRUCT);
			break;
		case FIR_ARRAY:
			Data.ARRAY = new FIRArray(*other.Data.ARRAY);
			break;
		case FIR_ANY:
			Data.ANY = new FIRAny(*other.Data.ANY);
			break;
		default:
			Data = other.Data;
			break;
		}
		return *this;
	}

	FORCEINLINE ~FFIRAnyValue() {
		switch (Type) {
		case FIR_STR:
			delete Data.STRING;
			break;
		case FIR_OBJ:
			delete Data.OBJECT;
			break;
		case FIR_TRACE:
			delete Data.TRACE;
			break;
		case FIR_STRUCT:
			delete Data.STRUCT;
			break;
		case FIR_ARRAY:
			delete Data.ARRAY;
			break;
		case FIR_ANY:
			delete Data.ANY;
			break;
		default:
			break;
		}
	}

	/**
	 * Allows you to get the type of the network value.
	 *
	 * @return	the type of the value
	 */
	FORCEINLINE EFIRValueType GetType() const {
		return Type;
	}

	/**
	 * Returns the the network value as integer.
	 * Asserts if the type is not int.
	 *
	 * @return	the stored integer
	 */
	FORCEINLINE FIRInt GetInt() const {
		switch (GetType()) {
		case FIR_INT:
			return Data.INT;
		case FIR_FLOAT:
			return (FIRInt) Data.FLOAT;
		case FIR_NIL:
			return 0;
		case FIR_BOOL:
			return (FIRInt) Data.BOOL;
		default:
			return 0;
		}
	}

	/**
	 * Returns the the network value as a float.
	 * Asserts if the type is not float.
	 *
	 * @return	the stored float
	 */
	FORCEINLINE FIRFloat GetFloat() const {
		switch (GetType()) {
		case FIR_FLOAT:
			return Data.FLOAT;
		case FIR_INT:
			return (FIRFloat) Data.INT;
		case FIR_BOOL:
			return (FIRFloat) Data.BOOL;
		case FIR_NIL:
			return 0.0;
		default:
			return 0.0;
		}
	}

	/**
	 * Returns the the network value as bool.
	 * Asserts if the type is not bool.
	 *
	 * @return	the stored bool
	 */
	FORCEINLINE FIRBool GetBool() const {
		switch (GetType()) {
		case FIR_FLOAT:
			return (FIRBool) Data.FLOAT;
		case FIR_INT:
			return (FIRBool) Data.INT;
		case FIR_NIL:
			return false;
		case FIR_BOOL:
			return Data.BOOL;
		case FIR_OBJ:
			return (FIRBool) Data.OBJECT;
		case FIR_TRACE:
			return (FIRBool) Data.TRACE->IsValid();
		default:
			return false;
		}
	}

	/**
	 * Returns the the network value as a class.
	 * Asserts if the type is not class.
	 *
	 * @return	the stored class
	 */
	FORCEINLINE FIRClass GetClass() const {
		switch (GetType()) {
		case FIR_CLASS:
			return Data.CLASS;
		default:
			return nullptr;
		}
	}

	/**
	 * Returns the the network value as string.
	 * Asserts if the type is not string.
	 *
	 * @return	the stored string
	 */
	FORCEINLINE const FIRStr& GetString() const {
		return *Data.STRING;
	}

	/**
	 * Returns the the network value as an object.
	 * Asserts if the type is not object.
	 *
	 * @return	the stored object
	 */
	FORCEINLINE FIRObj GetObj() const {
		switch (GetType()) {
		case FIR_OBJ:
			return *Data.OBJECT;
		case FIR_TRACE:
			return **Data.TRACE;
		default:
			return nullptr;
		}
	}

	/**
	 * Returns the the network value as a network trace.
	 * Asserts if the type is not trace.
	 *
	 * @return	the stored trace
	 */
	FORCEINLINE FIRTrace GetTrace() const {
		switch (GetType()) {
		case FIR_TRACE:
			return *Data.TRACE;
		case FIR_OBJ:
			return FIRTrace(Data.OBJECT->Get());
		default:
			return FIRTrace();
		}
	}

	/**
	 * Returns the the network value as a struct holder.
	 * Asserts if the type is not struct.
	 *
	 * @return	the stored struct
	 */
	FORCEINLINE const FIRStruct& GetStruct() const {
		return *Data.STRUCT;
	}

	/**
	 * Returns the network value as a network array.
	 * Asserts if the type not array.
	 *
	 * @return the stored array
	 */
	FORCEINLINE const FIRArray& GetArray() const {
		return *Data.ARRAY;
	}

	/**
	 * Returns the the network value as a any value.
	 * Asserts if the type is not any.
	 *
	 * @return	the stored trace
	 */
	FORCEINLINE const FIRAny& GetAny() const {
		return *Data.ANY;
	}

	bool Serialize(FStructuredArchive::FSlot Slot);

private:
	TEnumAsByte<EFIRValueType> Type = FIR_NIL;
	
	union {
		FIRInt		INT;
		FIRFloat	FLOAT;
		FIRBool		BOOL;
		FIRClass	CLASS;
		FIRStr*		STRING;
		FIRObj*		OBJECT;
		FIRTrace*	TRACE;
		FIRStruct*	STRUCT;
		FIRArray*	ARRAY;
		FIRAny*		ANY;
	} Data;
};

FORCEINLINE void operator<<(FStructuredArchive::FSlot Slot, FFIRAnyValue& AnyValue) {
	AnyValue.Serialize(Slot);
}

template<>
struct TStructOpsTypeTraits<FFIRAnyValue> : TStructOpsTypeTraitsBase2<FFIRAnyValue> {
	enum {
		WithStructuredSerializer = true,
		WithCopy = true,
	};
};

template<>
struct TMoveSupportTraits<FFIRAnyValue> : TMoveSupportTraitsBase<FFIRAnyValue, const FFIRAnyValue&> {
};

FICSITREFLECTION_API FString FIRObjectToString(UObject* InObj);

FICSITREFLECTION_API FString FIRClassToString(UClass* InClass);

FICSITREFLECTION_API FORCEINLINE FFIRAnyValue FIRCastValue(const FFIRAnyValue& Value, EFIRValueType ToType) {
	if (Value.GetType() == FIR_ANY) return FIRCastValue(Value.GetAny(), ToType);

	switch (ToType) {
	case FIR_BOOL:
		switch (Value.GetType()) {
		case FIR_NIL:
			return false;
		case FIR_BOOL:
			return Value;
		case FIR_INT:
			return Value.GetInt() != 0;
		case FIR_FLOAT:
			return Value.GetFloat() != 0.0;
		case FIR_STR:
			return Value.GetString() == TEXT("true");
		case FIR_OBJ:
			return Value.GetObj().IsValid();
		case FIR_CLASS:
			return Value.GetClass() != nullptr;
		case FIR_TRACE:
			return Value.GetTrace().IsValid();
		case FIR_ARRAY:
			return Value.GetArray().Num() > 0;
		default: ;
		}
		break;
	case FIR_INT:
		switch (Value.GetType()) {
		case FIR_NIL:
			return 1ll;
		case FIR_BOOL:
			return Value.GetBool() ? 1ll : 0ll;
		case FIR_INT:
			return Value;
		case FIR_FLOAT:
			return (FIRInt)Value.GetFloat();
		case FIR_STR: {
			FIRInt ValInt = 0;
			FDefaultValueHelper::ParseInt64(Value.GetString(), ValInt);
			return ValInt;
		} default: ;
		}
		break;
	case FIR_FLOAT:
		switch (Value.GetType()) {
		case FIR_NIL:
			return 0.0;
		case FIR_BOOL:
			return Value.GetBool() ? 1.0 : 0.0;
		case FIR_INT:
			return (double)Value.GetInt();
		case FIR_FLOAT:
			return Value;
		case FIR_STR: {
			FIRFloat ValFloat = 0.0f;
			FDefaultValueHelper::ParseDouble(Value.GetString(), ValFloat);
			return ValFloat;
		} default: ;
		}
		break;
	case FIR_STR:
		switch (Value.GetType()) {
		case FIR_NIL:
			return FString(TEXT("Nil"));
		case FIR_BOOL:
			return FString(Value.GetBool() ? TEXT("true") : TEXT("false"));
		case FIR_INT:
			return FString::Printf(TEXT("%lld"), Value.GetInt());
		case FIR_FLOAT:
			return FString::Printf(TEXT("%lg"), Value.GetFloat());
		case FIR_STR:
			return Value;
		case FIR_OBJ:
			return FIRObjectToString(Value.GetObj().Get());
		case FIR_CLASS:
			return FIRClassToString(Value.GetClass());
		case FIR_TRACE:
			return FIRObjectToString(*Value.GetTrace());
		default: ;
		}
		break;
	case FIR_OBJ:
		switch (Value.GetType()) {
		case FIR_OBJ:
			return Value;
		case FIR_NIL:
			return FWeakObjectPtr((UObject*)nullptr);
		case FIR_CLASS:
			return FWeakObjectPtr((UObject*)Value.GetClass());
		case FIR_TRACE:
			return FWeakObjectPtr(*Value.GetTrace());
		default: ;
		}
		break;
	case FIR_CLASS:
		switch (Value.GetType()) {
		case FIR_NIL:
			return (UClass*)nullptr;
		case FIR_OBJ:
			return Cast<UClass>(Value.GetObj().Get());
		case FIR_CLASS:
			return Value;
		case FIR_TRACE:
			return Cast<UClass>(*Value.GetTrace());
		default: ;
		}
		break;
	case FIR_TRACE:
		switch (Value.GetType()) {
		case FIR_NIL:
			return FFIRTrace();
		case FIR_OBJ:
			return FFIRTrace(Value.GetObj().Get());
		case FIR_CLASS:
			return FFIRTrace(Value.GetClass());
		case FIR_TRACE:
			return Value;
		default: ;
		}
		break;
	case FIR_STRUCT:
		switch (Value.GetType()) {
		case FIR_STRUCT:
			return Value;
		default: ;
		}
		break;
	case FIR_ARRAY:
		switch (Value.GetType()) {
		case FIR_ARRAY:
		default: ;
		}
		break;
	default: ;
	}
	return FIRAny();
}
