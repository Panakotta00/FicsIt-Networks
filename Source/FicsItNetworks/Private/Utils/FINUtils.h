#pragma once

struct FFINUtils {
	static FString TextRange(const FString& Text, const FTextRange& Range) {
		return Text.Mid(Range.BeginIndex, Range.Len());
	}
};