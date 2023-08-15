#pragma once

#include "FINReflectionSource.h"
#include "FINUReflectionSource.generated.h"

class UFINReflection;
UCLASS()
class FICSITNETWORKS_API UFINUReflectionSource : public UFINReflectionSource {
	GENERATED_BODY()
protected:
	struct FFINTypeMeta {
		FString InternalName;
		FText DisplayName;
		FText Description;
		TMap<FString, FString> PropertyInternalNames;
		TMap<FString, FText> PropertyDisplayNames;
		TMap<FString, FText> PropertyDescriptions;
		TMap<FString, int> PropertyRuntimes;
	};

	struct FFINFunctionMeta {
		FString InternalName;
		FText DisplayName;
		FText Description;
		TArray<FString> ParameterInternalNames;
		TArray<FText> ParameterDescriptions;
		TArray<FText> ParameterDisplayNames;
		int Runtime = 1;
	};

	struct FFINSignalMeta {
		FString InternalName;
		FText DisplayName;
		FText Description;
		TArray<FString> ParameterInternalNames;
		TArray<FText> ParameterDescriptions;
		TArray<FText> ParameterDisplayNames;
	};
	
	FFINTypeMeta GetClassMeta(UClass* Class) const;
	FFINFunctionMeta GetFunctionMeta(UClass* Class, UFunction* Func) const;
	FFINSignalMeta GetSignalMeta(UClass* Class, UFunction* Func) const;
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

	UFINFunction* GenerateFunction(FFINReflection* Ref, UClass* Class, UFunction* Func) const;
	UFINProperty* GenerateProperty(FFINReflection* Ref, const FFINTypeMeta& Meta, UClass* Class, FProperty* Prop) const;
	UFINProperty* GenerateProperty(FFINReflection* Ref, const FFINTypeMeta& Meta, UClass* Class, UFunction* Get) const;
	UFINSignal* GenerateSignal(FFINReflection* Ref, UClass* Class, UFunction* Func) const;
	static UFINSignal* GetSignalFromFunction(UFunction* Func);
	void SetupFunctionAsSignal(FFINReflection* Ref, UFunction* Func) const;
	static bool CheckName(const FString& Name);
};
