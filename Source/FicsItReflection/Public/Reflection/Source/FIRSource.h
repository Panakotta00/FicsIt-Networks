#pragma once

#include "CoreMinimal.h"
#include "Reflection/FIRClass.h"
#include "Reflection/FIRStruct.h"
#include "FIRSource.generated.h"

struct FFicsItReflectionModule;
UCLASS()
class FICSITREFLECTION_API UFIRSource : public UObject {
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
	virtual void FillData(FFicsItReflectionModule* Ref, UFIRClass* ToFillClass, UClass* Class) const {}
	virtual void FillData(FFicsItReflectionModule* Ref, UFIRStruct* ToFillStruct, UScriptStruct* Struct) const {}
};
