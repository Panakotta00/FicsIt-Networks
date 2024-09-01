#pragma once

#include "CoreMinimal.h"
#include "FIRSource.h"
#include "FIRSourceUObject.generated.h"

UENUM(BlueprintType)
enum EFIRMetaRuntimeState {
	Synchronous = 0,
	Parallel = 1,
	Asynchronous = 2,
	None = 3,
};

USTRUCT(BlueprintType)
struct FFIRBaseMeta {
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString InternalName;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FText Description;
};

USTRUCT(BlueprintType)
struct FFIRPropertyMeta : public FFIRBaseMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EFIRMetaRuntimeState> RuntimeState = EFIRMetaRuntimeState::None;
};

USTRUCT(BlueprintType)
struct FFIRFunctionParameterMeta : public FFIRBaseMeta {
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FFIRFunctionMeta : public FFIRBaseMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FFIRFunctionParameterMeta> Parameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EFIRMetaRuntimeState> RuntimeState = EFIRMetaRuntimeState::None;
};

USTRUCT(BlueprintType)
struct FFIRSignalMeta : public FFIRBaseMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FFIRFunctionParameterMeta> Parameters;
};

USTRUCT(BlueprintType)
struct FFIRTypeMeta : public FFIRBaseMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FFIRPropertyMeta> Properties;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FFIRFunctionMeta> Functions;
};

USTRUCT(BlueprintType)
struct FFIRClassMeta : public FFIRTypeMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<FString, FFIRSignalMeta> Signals;
};

class UFicsItReflection;

UCLASS()
class FICSITNETWORKS_API UFIRSourceUObject : public UFIRSource {
	GENERATED_BODY()
protected:
	FFIRClassMeta GetClassMeta(UClass* Class) const;
	FFIRFunctionMeta GetFunctionMeta(UClass* Class, UFunction* Func) const;
	FFIRSignalMeta GetSignalMeta(UClass* Class, UFunction* Func) const;
	FString GetFunctionNameFromUFunction(UFunction* Func) const;
	FString GetPropertyNameFromUFunction(UFunction* Func) const;
	FString GetPropertyNameFromFProperty(FProperty* Prop, bool& bReadOnly) const;
	FString GetSignalNameFromUFunction(UFunction* Func) const;

	static TMap<UFunction*, UFIRSignal*> FuncSignalMap;
	
public:
	// Begin UFINReflectionSource
	virtual bool ProvidesRequirements(UClass* Class) const override;
	virtual bool ProvidesRequirements(UScriptStruct* Struct) const override;
	virtual void FillData(FFicsItReflectionModule* Ref, UFIRClass* ToFillClass, UClass* Class) const override;
	virtual void FillData(FFicsItReflectionModule* Ref, UFIRStruct* ToFillStruct, UScriptStruct* Struct) const override;
	// End UFINReflectionSource

	UFIRFunction* GenerateFunction(FFicsItReflectionModule* Ref, const FFIRTypeMeta& Meta, UClass* Class, UFunction* Func) const;
	UFIRProperty* GenerateProperty(FFicsItReflectionModule* Ref, const FFIRTypeMeta& Meta, UClass* Class, FProperty* Prop) const;
	UFIRProperty* GenerateProperty(FFicsItReflectionModule* Ref, const FFIRTypeMeta& Meta, UClass* Class, UFunction* Get) const;
	UFIRSignal* GenerateSignal(FFicsItReflectionModule* Ref, const FFIRClassMeta& Meta, UClass* Class, UFunction* Func) const;
	static UFIRSignal* GetSignalFromFunction(UFunction* Func);
	void SetupFunctionAsSignal(FFicsItReflectionModule* Ref, UFunction* Func) const;
	static bool CheckName(const FString& Name);
};
