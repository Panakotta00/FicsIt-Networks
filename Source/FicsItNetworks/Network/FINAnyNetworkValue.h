#pragma once

#include "FINNetworkValues.h"
#include "FINValueReader.h"

#include "FINAnyNetworkValue.generated.h"

/**
 * This sturcture allows you to store any kind of network value.
 */
USTRUCT(BlueprintType)
struct FFINAnyNetworkValue {
	GENERATED_BODY()

	FFINAnyNetworkValue();

	FFINAnyNetworkValue(FINInt e);

	FFINAnyNetworkValue(FINFloat e);

	FFINAnyNetworkValue(FINBool e);

	FFINAnyNetworkValue(FINClass e);

	FFINAnyNetworkValue(const FINStr& e);

	FFINAnyNetworkValue(const FINObj& e);

	FFINAnyNetworkValue(const FINTrace& e);

	FFINAnyNetworkValue(const FINStruct& e);

	FFINAnyNetworkValue(const FINArray& e);

	FFINAnyNetworkValue(const FFINAnyNetworkValue& other);

	FFINAnyNetworkValue& operator=(const FFINAnyNetworkValue& other);

	~FFINAnyNetworkValue();

	/**
	 * Allows you to get the type of the network value.
	 *
	 * @return	the type of the value
	 */
	EFINNetworkValueType GetType() const {
		return Type;
	}

	/**
	 * Returns the the network value as integer.
	 * Asserts if the type is not int.
	 *
	 * @return	the stored integer
	 */
	FINInt GetInt() const {
		return Data.INT;
	}

	/**
	 * Returns the the network value as a float.
	 * Asserts if the type is not float.
	 *
	 * @return	the stored float
	 */
	FINFloat GetFloat() const {
		return Data.FLOAT;
	}

	/**
	 * Returns the the network value as bool.
	 * Asserts if the type is not bool.
	 *
	 * @return	the stored bool
	 */
	FINBool GetBool() const {
		return Data.BOOL;
	}

	/**
	 * Returns the the network value as a class.
	 * Asserts if the type is not class.
	 *
	 * @return	the stored class
	 */
	FINClass GetClass() const {
		return Data.CLASS;
	}

	/**
	 * Returns the the network value as string.
	 * Asserts if the type is not string.
	 *
	 * @return	the stored string
	 */
	const FINStr& GetString() const {
		return *Data.STRING;
	}

	/**
	 * Returns the the network value as an object.
	 * Asserts if the type is not object.
	 *
	 * @return	the stored object
	 */
	const FINObj& GetObj() const {
		return *Data.OBJECT;
	}

	/**
	 * Returns the the network value as a network trace.
	 * Asserts if the type is not trace.
	 *
	 * @return	the stored trace
	 */
	const FINTrace& GetTrace() const {
		return *Data.TRACE;
	}

	/**
	 * Returns the the network value as a struct holder.
	 * Asserts if the type is not struct.
	 *
	 * @return	the stored struct
	 */
	const FINStruct& GetStruct() const {
		return *Data.STRUCT;
	}

	/**
	 * Returns the network value as a network array.
	 * Asserts if the type not array.
	 *
	 * @return the stored array
	 */
	const FINArray& GetArray() const {
		return *Data.ARRAY;
	}

	bool Serialize(FArchive& Ar);

	void operator>>(FFINValueReader& Reader) const;

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
	} Data;
};

inline bool operator<<(FArchive& Ar, FFINAnyNetworkValue& Val) {
	return Val.Serialize(Ar);
}

template<>
struct TStructOpsTypeTraits<FFINAnyNetworkValue> : TStructOpsTypeTraitsBase2<FFINAnyNetworkValue> {
	enum {
		WithSerializer = true,
    };
};
