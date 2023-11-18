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

	FORCEINLINE static FString ObjectReferenceText(UFINClass* Class) {
		if (!Class) Class = Get()->FindClass(UObject::StaticClass());
		return FString::Printf(TEXT("Object<%s>"), *Class->GetInternalName());
	}

	FORCEINLINE static FString TraceReferenceText(UFINClass* Class) {
		if (!Class) Class = Get()->FindClass(UObject::StaticClass());
		return FString::Printf(TEXT("Trace<%s>"), *Class->GetInternalName());
	}

	FORCEINLINE static FString ClassReferenceText(UFINClass* Class) {
		if (!Class) Class = Get()->FindClass(UObject::StaticClass());
		return FString::Printf(TEXT("Class<%s>"), *Class->GetInternalName());
	}

	FORCEINLINE static FString StructReferenceText(UFINStruct* Type) {
		if (!Type) return TEXT("Struct");
		return FString::Printf(TEXT("Struct<%s>"), *Type->GetInternalName());
	}
};

UFINProperty* FINCreateFINPropertyFromFProperty(FProperty* Property, FProperty* OverrideProperty, UObject* Outer);
inline UFINProperty* FINCreateFINPropertyFromFProperty(FProperty* Property, UObject* Outer) {
	return FINCreateFINPropertyFromFProperty(Property, Property, Outer);
}
