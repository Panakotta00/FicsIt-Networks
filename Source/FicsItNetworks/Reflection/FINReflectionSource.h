#pragma once

#include "CoreMinimal.h"
#include "FINClass.h"
#include "FINStruct.h"
#include "FINReflectionSource.generated.h"

struct FFINReflection;
UCLASS()
class UFINReflectionSource : public UObject {
	GENERATED_BODY()
public:
	/**
	 * Checks if the reflection sources is capable of providing all required data
	 * for the given class.
	 * Requirements:
	 * - A internal name
	 * - Any additional special thing (function, property, signal, description, ...)
	 */
	virtual bool ProvidesRequirements(UClass* Class) const { return false; }
	virtual bool ProvidesRequirements(UScriptStruct* Struct) const { return false; }

	/**
	 * Fills the given FINType with data referenced by the given UType
	 */
	virtual void FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const {}
	virtual void FillData(FFINReflection* Ref, UFINStruct* ToFillStruct, UScriptStruct* Struct) const {}
};
