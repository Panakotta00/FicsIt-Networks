#pragma once

#include "FINReflectionSource.h"
#include "FINUReflectionSource.generated.h"

class UFINReflection;
UCLASS()
class UFINUReflectionSource : public UFINReflectionSource {
	GENERATED_BODY()
protected:
	struct FFINTypeMeta {
		FString InternalName;
		FText DisplayName;
		FText Description;
		TMap<FString, FString> PropertyInternalNames;
		TMap<FString, FText> PropertyDisplayNames;
		TMap<FString, FText> PropertyDescriptions;
	};

	struct FFINFunctionMeta {
		FString InternalName;
		FText DisplayName;
		FText Description;
		TArray<FString> ParameterInternalNames;
		TArray<FText> ParameterDescriptions;
		TArray<FText> ParameterDisplayNames;
	};
	
	FFINTypeMeta GetClassMeta(UClass* Class) const;
	FFINFunctionMeta GetFunctionMeta(UClass* Class, UFunction* Func) const;
	FString GetFunctionNameFromUFunction(UFunction* Func) const;
	FString GetPropertyNameFromUFunction(UFunction* Func) const;
	FString GetPropertyNameFromUProperty(UProperty* Prop, bool& bReadOnly) const;
	
public:
	// Begin UFINReflectionSource
	virtual bool ProvidesRequirements(UClass* Class) const override;
	virtual void FillData(FFINReflection* Ref, UFINClass* ToFillClass, UClass* Class) const override;
	// End UFINReflectionSource

	UFINFunction* GenerateFunction(UClass* Class, UFunction* Func) const;
	UFINProperty* GenerateProperty(const FFINTypeMeta& Meta, UClass* Class, UProperty* Prop) const;
	UFINProperty* GenerateProperty(const FFINTypeMeta& Meta, UClass* Class, UFunction* Get) const;
};
