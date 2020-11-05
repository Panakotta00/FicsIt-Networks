#pragma once

#include "FINClass.h"
#include "FINReflectionSource.h"

#include "FINReflection.generated.h"

UCLASS(BlueprintType)
class UFINReflection : public UObject {
	GENERATED_BODY()
private:
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	static UFINClass* FindClass(UClass* Clazz, bool bRecursive = true);
};

struct FFINReflection {
private:
	TMap<UClass*, UFINClass*> Classes;
	TArray<const UFINReflectionSource*> Sources;
	
public:
	static FFINReflection* Get();
	
	void PopulateSources();
	void LoadAllClasses();
	UFINClass* FindClass(UClass* Clazz, bool bRecursive = true, bool bTryToReflect = true);
	void PrintReflection();
	inline const TMap<UClass*, UFINClass*> GetClasses() { return Classes; }
};

UFINProperty* FINCreateFINPropertyFromUProperty(UProperty* Property, UProperty* OverrideProperty);
inline UFINProperty* FINCreateFINPropertyFromUProperty(UProperty* Property) {
	return FINCreateFINPropertyFromUProperty(Property, Property);
}
