#include "Utils/FINUtils.h"

const FRegexPattern UFINUtils::VariablePattern(TEXT("\\${(\\w+)(\\|(\\w+))?}"));

FString UFINUtils::InterpolateString(const FString& Text, TMap<FString, FString> Variables, bool bEmptyInvalidVariables) {
	FString OutText;
	FRegexMatcher Matcher(VariablePattern, Text);
	int CurrentIndex = 0;
	while (Matcher.FindNext()) {
		FString Variable = Matcher.GetCaptureGroup(1);
		FString* Value = Variables.Find(Variable);
		if (Value || bEmptyInvalidVariables) {
			OutText.Append(TextRange(Text, FTextRange(CurrentIndex, Matcher.GetBeginLimit())));
			if (Value) OutText.Append(*Value);
		} else {
			OutText.Append(TextRange(Text, FTextRange(CurrentIndex, Matcher.GetEndLimit())));
		} 
		CurrentIndex = Matcher.GetEndLimit();
	}
	OutText.Append(Text.RightChop(CurrentIndex));
	return OutText;
}
