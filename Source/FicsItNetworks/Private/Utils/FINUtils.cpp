#include "Utils/FINUtils.h"

#include "Internationalization/Regex.h"

const FRegexPattern UFINUtils::VariablePattern(TEXT("\\$\\{(\\w+)(\\|(\\w+))?\\}"));

#pragma optimize("", off)
FString UFINUtils::InterpolateString(const FString& Text, const TMap<FString, FString>& Variables, bool bEmptyInvalidVariables) {
	FString OutText;
	FRegexMatcher Matcher(VariablePattern, Text);
	int CurrentIndex = 0;
	while (Matcher.FindNext()) {
		FString Variable = Matcher.GetCaptureGroup(1);
		const FString* Value = Variables.Find(Variable);
		if (Value || bEmptyInvalidVariables) {
			OutText.Append(TextRange(Text, FTextRange(CurrentIndex, Matcher.GetMatchBeginning())));
			if (Value) OutText.Append(*Value);
		} else {
			OutText.Append(TextRange(Text, FTextRange(CurrentIndex, Matcher.GetMatchEnding())));
		} 
		CurrentIndex = Matcher.GetMatchEnding();
	}
	OutText.Append(Text.RightChop(CurrentIndex));
	return OutText;
}
#pragma optimize("", on)

void UFINUtils::VariablesFormString(const FString& Text, TMap<FString, FString>& OutVariables) {
	FRegexMatcher Matcher(VariablePattern, Text);
	while (Matcher.FindNext()) {
		FString Variable = Matcher.GetCaptureGroup(1);
		FString Default = Matcher.GetCaptureGroup(3);
		OutVariables.Add(Variable, Default);
	}
}
