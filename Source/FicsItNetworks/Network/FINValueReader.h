#pragma once

#include "CoreMinimal.h"
#include "FINNetworkTrace.h"

/**
 * Base class of value readers which provides an abstract interface
 * allowing to read a network type and convert it to a different value types.
 * f.e. conversion of UFunction parameter list to lua values
 */
class FFINValueReader {
public:
	virtual ~FFINValueReader() {}

	/** Writes a string to the signal reader */
	virtual void operator<<(const FString& Str) = 0;

	/** Writes a double to the signal reader */
	virtual void operator<<(double Num) = 0;

	/** Writes a integer to the signal reader */
	virtual void operator<<(int Num) = 0;

	/** Writes a boolean to the signal reader */
	virtual void operator<<(bool B) = 0;

	/** Writes a object instance to the signal reader */
	virtual void operator<<(UObject* Obj) = 0;

	/** Writes a object referenced by a network trace to the signal reader */
	virtual void operator<<(const FFINNetworkTrace& Obj) = 0;

	/** Trys to write the given object identified by the given type id to the signal reader */
	virtual void WriteAbstract(const void* obj, const FString& id) {};
};
