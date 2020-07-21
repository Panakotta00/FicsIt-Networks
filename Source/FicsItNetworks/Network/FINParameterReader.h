#pragma once

#include "CoreMinimal.h"
#include "FINNetworkTrace.h"

class FFINParameterReader {
public:
	virtual ~FFINParameterReader() {}

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
	virtual void operator<<(const FFINNetworkTrace& Obj);

	/** Trys to write the given object identified by the given type id to the signal reader */
	virtual void WriteAbstract(const void* obj, const FString& id);
};
