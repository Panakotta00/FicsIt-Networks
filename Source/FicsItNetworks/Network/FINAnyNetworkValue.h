#pragma once

#include "FINNetworkComponent.h"
#include "FINNetworkValues.h"
#include "Misc/DefaultValueHelper.h"
#include "FINAnyNetworkValue.generated.h"

/**
 * This sturcture allows you to store any kind of network value.
 */
USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINAnyNetworkValue {
	GENERATED_BODY()
	
	FORCEINLINE FFINAnyNetworkValue() : Type(FIN_NIL), Data() {}

	FORCEINLINE FFINAnyNetworkValue(EFINNetworkValueType InType) {
		Type = InType;
		switch (InType) {
		case FIN_NIL:
			break;
		case FIN_BOOL:
			Data.BOOL = false;
			break;
		case FIN_INT:
			Data.INT = 0;
			break;
		case FIN_FLOAT:
			Data.FLOAT = 0.0f;
			break;
		case FIN_STR:
			Data.STRING = new FINStr();
			break;
		case FIN_OBJ:
			Data.OBJECT = new FINObj();
			break;
		case FIN_CLASS:
			Data.CLASS = nullptr;
			break;
		case FIN_TRACE:
			Data.TRACE = new FINTrace();
			break;
		case FIN_STRUCT:
			Data.STRUCT = new FINStruct();
			break;
		case FIN_ARRAY:
			Data.ARRAY = new FINArray();
			break;
		case FIN_ANY:
			Data.ANY = new FINAny();
			break;
		}
	}

	FORCEINLINE FFINAnyNetworkValue(FINInt e) {
		Data.INT = e;
		Type = FIN_INT;
	}

	FORCEINLINE FFINAnyNetworkValue(FINFloat e) {
		Data.FLOAT = e;
		Type = FIN_FLOAT;
	}

	FORCEINLINE FFINAnyNetworkValue(FINBool e) {
		Data.BOOL = e;
		Type = FIN_BOOL;
	}

	FORCEINLINE FFINAnyNetworkValue(FINClass e) {
		Data.CLASS = e;
		Type = FIN_CLASS;
	}

	FORCEINLINE FFINAnyNetworkValue(const FINStr& e) {
		Data.STRING = new FINStr(e);
		Type = FIN_STR;
	}

	FORCEINLINE FFINAnyNetworkValue(const FINObj& e) {
		Data.OBJECT = new FINObj(e);
		Type = FIN_OBJ;
	}

	FORCEINLINE FFINAnyNetworkValue(const FINTrace& e) {
		Data.TRACE = new FINTrace(e);
		Type = FIN_TRACE;
	}

	FORCEINLINE FFINAnyNetworkValue(const FINStruct& e) {
		Data.STRUCT = new FINStruct(e);
		Type = FIN_STRUCT;
	}

	FORCEINLINE FFINAnyNetworkValue(const FINArray& e) {
		Data.ARRAY = new FINArray(e);
		Type = FIN_ARRAY;
	}

	FORCEINLINE FFINAnyNetworkValue(const FFINAnyNetworkValue& other) {
		*this = other;
	}

	FORCEINLINE FFINAnyNetworkValue& operator=(const FFINAnyNetworkValue& other) {
		this->~FFINAnyNetworkValue();
		Type = other.Type;
		switch (Type) {
		case FIN_STR:
			Data.STRING = new FINStr(*other.Data.STRING);
			break;
		case FIN_OBJ:
			Data.OBJECT = new FINObj(*other.Data.OBJECT);
			break;
		case FIN_TRACE:
			Data.TRACE = new FINTrace(*other.Data.TRACE);
			break;
		case FIN_STRUCT:
			Data.STRUCT = new FINStruct(*other.Data.STRUCT);
			break;
		case FIN_ARRAY:
			Data.ARRAY = new FINArray(*other.Data.ARRAY);
			break;
		case FIN_ANY:
			Data.ANY = new FINAny(*other.Data.ANY);
			break;
		default:
			Data = other.Data;
			break;
		}
		return *this;
	}

	FORCEINLINE ~FFINAnyNetworkValue() {
		switch (Type) {
		case FIN_STR:
			delete Data.STRING;
			break;
		case FIN_OBJ:
			delete Data.OBJECT;
			break;
		case FIN_TRACE:
			delete Data.TRACE;
			break;
		case FIN_STRUCT:
			delete Data.STRUCT;
			break;
		case FIN_ARRAY:
			delete Data.ARRAY;
			break;
		case FIN_ANY:
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
	FORCEINLINE EFINNetworkValueType GetType() const {
		return Type;
	}

	/**
	 * Returns the the network value as integer.
	 * Asserts if the type is not int.
	 *
	 * @return	the stored integer
	 */
	FORCEINLINE FINInt GetInt() const {
		switch (GetType()) {
		case FIN_INT:
			return Data.INT;
		case FIN_FLOAT:
			return (FINInt) Data.FLOAT;
		case FIN_NIL:
			return 0;
		case FIN_BOOL:
			return (FINInt) Data.BOOL;
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
	FORCEINLINE FINFloat GetFloat() const {
		switch (GetType()) {
		case FIN_FLOAT:
			return Data.FLOAT;
		case FIN_INT:
			return (FINFloat) Data.INT;
		case FIN_BOOL:
			return (FINFloat) Data.BOOL;
		case FIN_NIL:
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
	FORCEINLINE FINBool GetBool() const {
		return Data.BOOL;
	}

	/**
	 * Returns the the network value as a class.
	 * Asserts if the type is not class.
	 *
	 * @return	the stored class
	 */
	FORCEINLINE FINClass GetClass() const {
		return Data.CLASS;
	}

	/**
	 * Returns the the network value as string.
	 * Asserts if the type is not string.
	 *
	 * @return	the stored string
	 */
	FORCEINLINE const FINStr& GetString() const {
		return *Data.STRING;
	}

	/**
	 * Returns the the network value as an object.
	 * Asserts if the type is not object.
	 *
	 * @return	the stored object
	 */
	FORCEINLINE const FINObj& GetObj() const {
		return *Data.OBJECT;
	}

	/**
	 * Returns the the network value as a network trace.
	 * Asserts if the type is not trace.
	 *
	 * @return	the stored trace
	 */
	FORCEINLINE const FINTrace& GetTrace() const {
		return *Data.TRACE;
	}

	/**
	 * Returns the the network value as a struct holder.
	 * Asserts if the type is not struct.
	 *
	 * @return	the stored struct
	 */
	FORCEINLINE const FINStruct& GetStruct() const {
		return *Data.STRUCT;
	}

	/**
	 * Returns the network value as a network array.
	 * Asserts if the type not array.
	 *
	 * @return the stored array
	 */
	FORCEINLINE const FINArray& GetArray() const {
		return *Data.ARRAY;
	}

	/**
	 * Returns the the network value as a any value.
	 * Asserts if the type is not any.
	 *
	 * @return	the stored trace
	 */
	FORCEINLINE const FINAny& GetAny() const {
		return *Data.ANY;
	}

	bool Serialize(FArchive& Ar);

private:
	TEnumAsByte<EFINNetworkValueType> Type = FIN_NIL;
	
	union {
		FINInt		INT;
		FINFloat	FLOAT;
		FINBool		BOOL;
		FINClass	CLASS;
		FINStr*		STRING;
		FINObj*		OBJECT;
		FINTrace*	TRACE;
		FINStruct*	STRUCT;
		FINArray*	ARRAY;
		FINAny*		ANY;
	} Data;
};

inline bool operator<<(FArchive& Ar, FFINAnyNetworkValue& Val) {
	return Val.Serialize(Ar);
}

inline bool operator<<(FStructuredArchive::FSlot Slot, FFINAnyNetworkValue& Val) {
	return Val.Serialize(Slot.GetUnderlyingArchive());
}

template<>
struct TStructOpsTypeTraits<FFINAnyNetworkValue> : TStructOpsTypeTraitsBase2<FFINAnyNetworkValue> {
	enum {
		WithSerializer = true,
		WithCopy = true,
	};
};

template<>
struct TMoveSupportTraits<FFINAnyNetworkValue> : TMoveSupportTraitsBase<FFINAnyNetworkValue, const FFINAnyNetworkValue&> {
};

FString FINObjectToString(UObject* InObj);

FString FINClassToString(UClass* InClass);

FORCEINLINE FFINAnyNetworkValue FINCastNetworkValue(const FFINAnyNetworkValue& Value, EFINNetworkValueType ToType) {
	if (Value.GetType() == FIN_ANY) return FINCastNetworkValue(Value.GetAny(), ToType);
	
	switch (ToType) {
	case FIN_BOOL:
		switch (Value.GetType()) {
		case FIN_NIL:
			return false;
		case FIN_BOOL:
			return Value;
		case FIN_INT:
			return Value.GetInt() != 0;
		case FIN_FLOAT:
			return Value.GetFloat() != 0.0;
		case FIN_STR:
			return Value.GetString() == TEXT("true");
		case FIN_OBJ:
			return Value.GetObj().IsValid();
		case FIN_CLASS:
			return Value.GetClass() != nullptr;
		case FIN_TRACE:
			return Value.GetTrace().IsValid();
		case FIN_ARRAY:
			return Value.GetArray().Num() > 0;
		default: ;
		}
		break;
	case FIN_INT:
		switch (Value.GetType()) {
		case FIN_NIL:
			return 1ll;
		case FIN_BOOL:
			return Value.GetBool() ? 1ll : 0ll;
		case FIN_INT:
			return Value;
		case FIN_FLOAT:
			return (FINInt)Value.GetFloat();
		case FIN_STR: {
			FINInt ValInt = 0;
			FDefaultValueHelper::ParseInt64(Value.GetString(), ValInt);
			return ValInt;
		} default: ;
		}
		break;
	case FIN_FLOAT:
		switch (Value.GetType()) {
		case FIN_NIL:
			return 0.0;
		case FIN_BOOL:
			return Value.GetBool() ? 1.0 : 0.0;
		case FIN_INT:
			return (double)Value.GetInt();
		case FIN_FLOAT:
			return Value;
		case FIN_STR: {
			FINFloat ValFloat = 0.0f;
			FDefaultValueHelper::ParseDouble(Value.GetString(), ValFloat);
			return ValFloat;
		} default: ;
		}
		break;
	case FIN_STR:
		switch (Value.GetType()) {
		case FIN_NIL:
			return FString(TEXT("Nil"));
		case FIN_BOOL:
			return FString(Value.GetBool() ? TEXT("true") : TEXT("false"));
		case FIN_INT:
			return FString::Printf(TEXT("%lld"), Value.GetInt());
		case FIN_FLOAT:
			return FString::Printf(TEXT("%lg"), Value.GetFloat());
		case FIN_STR:
			return Value;
		case FIN_OBJ:
			return FINObjectToString(Value.GetObj().Get());
		case FIN_CLASS:
			return FINClassToString(Value.GetClass());
		case FIN_TRACE:
			return FINObjectToString(*Value.GetTrace());
		default: ;
		}
		break;
	case FIN_OBJ:
		switch (Value.GetType()) {
		case FIN_OBJ:
			return Value;
		case FIN_NIL:
			return FWeakObjectPtr((UObject*)nullptr);
		case FIN_CLASS:
			return FWeakObjectPtr((UObject*)Value.GetClass());
		case FIN_TRACE:
			return FWeakObjectPtr(*Value.GetTrace());
		default: ;
		}
		break;
	case FIN_CLASS:
		switch (Value.GetType()) {
		case FIN_NIL:
			return (UClass*)nullptr;
		case FIN_OBJ:
			return Cast<UClass>(Value.GetObj().Get());
		case FIN_CLASS:
			return Value;
		case FIN_TRACE:
			return Cast<UClass>(*Value.GetTrace());
		default: ;
		}
		break;
	case FIN_TRACE:
		switch (Value.GetType()) {
		case FIN_NIL:
			return FFINNetworkTrace();
		case FIN_OBJ:
			return FFINNetworkTrace(Value.GetObj().Get());
		case FIN_CLASS:
			return FFINNetworkTrace(Value.GetClass());
		case FIN_TRACE:
			return Value;
		default: ;
		}
		break;
	case FIN_STRUCT:
		switch (Value.GetType()) {
		case FIN_STRUCT:
			return Value;
		default: ;
		}
		break;
	case FIN_ARRAY:
		switch (Value.GetType()) {
		case FIN_ARRAY:
		default: ;
		}
		break;
	default: ;
	}
	return FINAny();
}
