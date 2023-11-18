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