#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FINUtils.generated.h"

UCLASS()
class UFINUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static FString InterpolateString(const FString& Text, const TMap<FString, FString>& Variables, bool bEmptyInvalidVariables);
	UFUNCTION(BlueprintCallable)
	static void VariablesFormString(const FString& Text, TMap<FString, FString>& OutVariables);
	
	static FString TextRange(const FString& Text, const FTextRange& Range) {
		return Text.Mid(Range.BeginIndex, Range.Len());
	}

private:
	static const FRegexPattern VariablePattern;
};


UENUM(Blueprintable)
enum EFINMetaRuntimeState {
	Synchronous = 0,
	Parallel = 1,
	Asynchronous = 2,
};


USTRUCT(Blueprintable)
struct FFINBlueprintPropertyMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString InternalName;
	
	UPROPERTY(BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite)
	FText Description;

	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<EFINMetaRuntimeState> RuntimeState = Parallel;
};


USTRUCT(Blueprintable)
struct FFINBlueprintFunctionMetaParameter {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString InternalName;
	
	UPROPERTY(BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite)
	FText Description;
};

USTRUCT(Blueprintable)
struct FFINBlueprintFunctionMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString InternalName;

	UPROPERTY(BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite)
	FText Description;

	UPROPERTY(BlueprintReadWrite)
	TArray<FFINBlueprintFunctionMetaParameter> Parameters;

	UPROPERTY(BlueprintReadWrite)
	TEnumAsByte<EFINMetaRuntimeState> RuntimeState;
};

USTRUCT(Blueprintable)
struct FFINBlueprintSignalMeta {
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString InternalName;
	
	UPROPERTY(BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite)
	FText Description;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<FFINBlueprintFunctionMetaParameter> Parameters;
};

