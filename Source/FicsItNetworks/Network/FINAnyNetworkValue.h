#pragma once

#include "FINNetworkValues.h"

#include "FINAnyNetworkValue.generated.h"

/**
 * This sturcture allows you to store any kind of network value.
 */
USTRUCT(BlueprintType)
struct FICSITNETWORKS_API FFINAnyNetworkValue {
	GENERATED_BODY()
	
	FORCEINLINE FFINAnyNetworkValue() : Type(FIN_NIL), Data() {}

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
		switch (GetType()) {
		case FIN_FLOAT:
			return (FINBool) Data.FLOAT;
		case FIN_INT:
			return (FINBool) Data.INT;
		case FIN_NIL:
			return false;
		case FIN_BOOL:
			return Data.BOOL;
		case FIN_OBJ:
			return (FINBool) Data.OBJECT;
		case FIN_TRACE:
			return (FINBool) Data.TRACE->IsValid();
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
	FORCEINLINE FINClass GetClass() const {
		switch (GetType()) {
		case FIN_CLASS:
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
	FORCEINLINE const FINStr& GetString() const {
		return *Data.STRING;
	}

	/**
	 * Returns the the network value as an object.
	 * Asserts if the type is not object.
	 *
	 * @return	the stored object
	 */
	FORCEINLINE FINObj GetObj() const {
		switch (GetType()) {
		case FIN_OBJ:
			return *Data.OBJECT;
		case FIN_TRACE:
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
	FORCEINLINE FINTrace GetTrace() const {
		switch (GetType()) {
		case FIN_TRACE:
			return *Data.TRACE;
		case FIN_OBJ:
			return FINTrace(Data.OBJECT->Get());
		default:
			return FINTrace();
		}
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
