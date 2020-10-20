#pragma once

#include "CoreMinimal.h"
#include "FINNetworkTrace.h"
#include "FINNetworkValues.h"

/**
 * Base class of value readers which provides an abstract interface
 * allowing to read a network type and convert it to a different value types.
 * f.e. conversion of UFunction parameter list to lua values
 */
class FFINValueReader {
public:
	virtual ~FFINValueReader() {}

	/** Writes nil to the signal reader */
	virtual void nil() = 0;

	/** Writes a boolean to the signal reader */
	virtual void operator<<(FINBool B) = 0;
	
	/** Writes a integer to the signal reader */
	virtual void operator<<(FINInt Num) = 0;

	/** Writes a double to the signal reader */
	virtual void operator<<(FINFloat Num) = 0;

	/** Writes a class to the signal reader */
	virtual void operator<<(FINClass Class) = 0;
	
	/** Writes a string to the signal reader */
	virtual void operator<<(const FINStr& Str) = 0;

	/** Writes a object instance to the signal reader */
	virtual void operator<<(const FINObj& Obj) = 0;

	/** Writes a object referenced by a network trace to the signal reader */
	virtual void operator<<(const FINTrace& Obj) = 0;

	/** Writes any kind of struct to the value reader */
	virtual void operator<<(const FINStruct& Struct) = 0;

	/** Writes a array to the signal reader */
	virtual void operator<<(const FINArray& Array) = 0;

	virtual void operator<<(UObject* Obj) {
		*this << FWeakObjectPtr(Obj);
	}
};

class FFINVoidValueReader : public FFINValueReader {
public:

	virtual void nil() override {}
	virtual void operator<<(FINBool B) override {}
	virtual void operator<<(FINInt Num) override {}
	virtual void operator<<(FINFloat Num) override {}
	virtual void operator<<(FINClass Class) override {}
	virtual void operator<<(const FINStr& Str) override {}
	virtual void operator<<(const FINObj& Obj) override {}
	virtual void operator<<(const FINTrace& Obj) override {}
	virtual void operator<<(const FINStruct& Struct) override {}
	virtual void operator<<(const FINArray& Array) override {}
};