#pragma once

#include "FINReflectionSource.h"
#include "FINUReflectionSource.generated.h"


UENUM(BlueprintType)
enum EFINReflectionMetaRuntimeState {
	Synchronous = 0,
	Parallel = 1,
	Asynchronous = 2,
	None = 3,
};

USTRUCT(BlueprintType)
struct FFINReflectionBaseMeta {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString InternalName;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText Description;
};

USTRUCT(BlueprintType)
struct FFINReflectionPropertyMeta : public FFINReflectionBaseMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EFINReflectionMetaRuntimeState> RuntimeState = EFINReflectionMetaRuntimeState::None;
};

USTRUCT(BlueprintType)
struct FFINReflectionFunctionParameterMeta : public FFINReflectionBaseMeta {
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FFINReflectionFunctionMeta : public FFINReflectionBaseMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FFINReflectionFunctionParameterMeta> Parameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EFINReflectionMetaRuntimeState> RuntimeState = EFINReflectionMetaRuntimeState::None;
};

USTRUCT(BlueprintType)
struct FFINReflectionSignalMeta : public FFINReflectionBaseMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FFINReflectionFunctionParameterMeta> Parameters;
};

USTRUCT(BlueprintType)
struct FFINReflectionTypeMeta : public FFINReflectionBaseMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FFINReflectionPropertyMeta> Properties;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FFINReflectionFunctionMeta> Functions;
};

USTRUCT(BlueprintType)
struct FFINReflectionClassMeta : public FFINReflectionTypeMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FFINReflectionSignalMeta> Signals;
};

class UFINReflection;
UCLASS()
class FICSITNETWORKS_API UFINUReflectionSource : public UFINReflectionSource {
	GENERATED_BODY()
protected:
	FFINReflectionClassMeta GetClassMeta(UClass* Class) const;
	FFINReflectionFunctionMeta GetFunctionMeta(UClass* Class, UFunction* Func) const;
	FFINReflectionSignalMeta GetSignalMeta(UClass* Class, UFunction* Func) const;
	FString GetFunctionNameFromUFunction(UFunction* Func) const;
	FString GetPropertyNameFromUFunction(UFunction* Func) const;
	FString GetPropertyNameFromFProperty(FProperty* Prop, bool& bReadOnly) const;
	FString GetSignalNameFromUFunction(UFunction* Func) const;

	static TMap<UFunction*, UFINSignal*> FuncSignalMap;
	
public:
	// Begin UFINReflectionSource
	virtual bool ProvidesRequirements(UClass* Class) const override;
	virtual bool ProvidesRequirements(UScriptStruct* Struct) const override;
	virtual void FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const override;
	virtual void FillData(FFINReflection* Ref, UFINStruct* ToFillStruct, UScriptStruct* Struct) const override;
	// End UFINReflectionSource

	UFINFunction* GenerateFunction(FFINReflection* Ref, const FFINReflectionTypeMeta& Meta, UClass* Class, UFunction* Func) const;
	UFINProperty* GenerateProperty(FFINReflection* Ref, const FFINReflectionTypeMeta& Meta, UClass* Class, FProperty* Prop) const;
	UFINProperty* GenerateProperty(FFINReflection* Ref, const FFINReflectionTypeMeta& Meta, UClass* Class, UFunction* Get) const;
	UFINSignal* GenerateSignal(FFINReflection* Ref, const FFINReflectionClassMeta& Meta, UClass* Class, UFunction* Func) const;
	static UFINSignal* GetSignalFromFunction(UFunction* Func);
	void SetupFunctionAsSignal(FFINReflection* Ref, UFunction* Func) const;
	static bool CheckName(const FString& Name);
};
