#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Util/SemVersion.h"
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

	template<typename T>
	static TArrayView<T> PaginateArray(const TArrayView<T>& Array, int64 PageSize, int64 Page) {
		PageSize = FMath::Max(0, PageSize);
		int64 Offset = Page*PageSize;
		if (Offset < 0) Offset = Array.Num() + Page*PageSize;
		int64 Num = FMath::Min(PageSize, Array.Num() - Offset);
		if (Offset < 0) {
			Num += Offset;
			Offset = 0;
		}
		if (Offset < 0 || Num < 0) {
			return TArrayView<T>();
		} else {
			return TArrayView<T>(Array.GetData() + Offset, Num);
		}
	}

	static FVersion GetFINSaveVersion(UObject* WorldContext);

private:
	static const FRegexPattern VariablePattern;
};
