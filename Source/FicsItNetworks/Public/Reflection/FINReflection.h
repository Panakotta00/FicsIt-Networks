#pragma once

#include "FINClass.h"
#include "FINReflectionSource.h"
#include "FINStruct.h"
#include "FINReflection.generated.h"

UCLASS(BlueprintType)
class FICSITNETWORKS_API UFINReflection : public UObject {
	GENERATED_BODY()
private:
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	static UFINClass* FindClass(UClass* Clazz, bool bRecursive = true);

	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	static UFINStruct* FindStruct(UScriptStruct* Struct, bool bRecursive = true);
};

struct FICSITNETWORKS_API FFINReflection {
private:
	TMap<UClass*, UFINClass*> Classes;
	TMap<UScriptStruct*, UFINStruct*> Structs;
	TArray<const UFINReflectionSource*> Sources;
	
public:
	static FFINReflection* Get();
	
	void PopulateSources();
	void LoadAllTypes();
	UFINClass* FindClass(UClass* Clazz, bool bRecursive = true, bool bTryToReflect = true);
	UFINStruct* FindStruct(UScriptStruct* Struct, bool bRecursive = true, bool bTryToReflect = true);
	void PrintReflection();
	inline const TMap<UClass*, UFINClass*>& GetClasses() { return Classes; }
	inline const TMap<UScriptStruct*, UFINStruct*>& GetStructs() { return Structs; }
};

UFINProperty* FINCreateFINPropertyFromUProperty(UProperty* Property, UProperty* OverrideProperty, UObject* Outer);
inline UFINProperty* FINCreateFINPropertyFromUProperty(UProperty* Property, UObject* Outer) {
	return FINCreateFINPropertyFromUProperty(Property, Property, Outer);
}
