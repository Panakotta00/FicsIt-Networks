#pragma once

#include "CoreMinimal.h"
#include "FIRAnyValue.h"
#include "Modules/ModuleManager.h"
#include "Reflection/FIRClass.h"
#include "FicsItReflection.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFicsItReflection, Warning, All);

UCLASS(BlueprintType)
class FICSITREFLECTION_API UFicsItReflection : public UObject {
	GENERATED_BODY()
private:
	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	static UFIRClass* FindClass(UClass* Clazz, bool bRecursive = true);

	UFUNCTION(BlueprintCallable, Category="Network|Reflection")
	static UFIRStruct* FindStruct(UScriptStruct* Struct, bool bRecursive = true);
};

class UFIRSource;

struct FICSITREFLECTION_API FFicsItReflectionModule : public IModuleInterface {
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	static FFicsItReflectionModule& Get();

	TMulticastDelegate<void(UObject*, class UFIRSignal*, const TArray<FFIRAnyValue>&)> OnSignalTriggered;
	
	void PopulateSources();
	void LoadAllTypes();
	
	UFIRClass* FindClass(UClass* Class, bool bRecursive = true, bool bTryToReflect = true);
	UFIRClass* FindClass(const FString& ClassName) const;
	UClass* FindUClass(UFIRClass* Class) const;
	UFIRStruct* FindStruct(UScriptStruct* Struct, bool bRecursive = true, bool bTryToReflect = true);
	UFIRStruct* FindStruct(const FString& StructName) const;
	UScriptStruct* FindScriptStruct(UFIRStruct* Struct) const;

	void PrintReflection();

	inline const TMap<UClass*, UFIRClass*>& GetClasses() { return Classes; }
	inline const TMap<UScriptStruct*, UFIRStruct*>& GetStructs() { return Structs; }

	FORCEINLINE static FString ObjectReferenceText(UFIRClass* Class) {
		if (!Class) Class = Get().FindClass(UObject::StaticClass());
		return FString::Printf(TEXT("Object<%s>"), *Class->GetInternalName());
	}

	FORCEINLINE static FString TraceReferenceText(UFIRClass* Class) {
		if (!Class) Class = Get().FindClass(UObject::StaticClass());
		return FString::Printf(TEXT("Trace<%s>"), *Class->GetInternalName());
	}

	FORCEINLINE static FString ClassReferenceText(UFIRClass* Class) {
		if (!Class) Class = Get().FindClass(UObject::StaticClass());
		return FString::Printf(TEXT("Class<%s>"), *Class->GetInternalName());
	}

	FORCEINLINE static FString StructReferenceText(UFIRStruct* Type) {
		if (!Type) return TEXT("Struct");
		return FString::Printf(TEXT("Struct<%s>"), *Type->GetInternalName());
	}

private:
	TMap<UClass*, UFIRClass*> Classes;
	TMap<UFIRClass*, UClass*> ClassesReversed;
	TMap<FString, UFIRClass*> ClassNames;

	TMap<UScriptStruct*, UFIRStruct*> Structs;
	TMap<UFIRStruct*, UScriptStruct*> StructsReversed;
	TMap<FString, UFIRStruct*> StructNames;

	TArray<UFIRSource*> Sources;
};
