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
	TMap<UFINClass*, UClass*> ClassesReversed;
	TMap<FString, UFINClass*> ClassNames;
	
	TMap<UScriptStruct*, UFINStruct*> Structs;
	TMap<UFINStruct*, UScriptStruct*> StructsReversed;
	TMap<FString, UFINStruct*> StructNames;
	
	TArray<const UFINReflectionSource*> Sources;
	
public:
	static FFINReflection* Get();
	
	void PopulateSources();
	void LoadAllTypes();
	
	UFINClass* FindClass(UClass* Class, bool bRecursive = true, bool bTryToReflect = true);
	UFINClass* FindClass(const FString& ClassName) const;
	UClass* FindUClass(UFINClass* Class) const;
	UFINStruct* FindStruct(UScriptStruct* Struct, bool bRecursive = true, bool bTryToReflect = true);
	UFINStruct* FindStruct(const FString& StructName) const;
	UScriptStruct* FindScriptStruct(UFINStruct* Struct) const;

	void PrintReflection();

	inline const TMap<UClass*, UFINClass*>& GetClasses() { return Classes; }
	inline const TMap<UScriptStruct*, UFINStruct*>& GetStructs() { return Structs; }
};

UFINProperty* FINCreateFINPropertyFromFProperty(FProperty* Property, FProperty* OverrideProperty, UObject* Outer);
inline UFINProperty* FINCreateFINPropertyFromFProperty(FProperty* Property, UObject* Outer) {
	return FINCreateFINPropertyFromFProperty(Property, Property, Outer);
}
