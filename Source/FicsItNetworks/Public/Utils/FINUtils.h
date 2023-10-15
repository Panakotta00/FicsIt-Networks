#pragma once

#include "CoreMinimal.h"
#include "FINUtils.generated.h"

UCLASS()
class UFINUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static FString InterpolateString(const FString& Text, TMap<FString, FString> Variables, bool bEmptyInvalidVariables);
	
	static FString TextRange(const FString& Text, const FTextRange& Range) {
		return Text.Mid(Range.BeginIndex, Range.Len());
	}

private:
	static const FRegexPattern VariablePattern;
};