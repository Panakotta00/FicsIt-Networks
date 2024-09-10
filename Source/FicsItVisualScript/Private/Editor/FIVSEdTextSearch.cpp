#include "Editor/FIVSEdTextSearch.h"

void FFIVSEdTextSearch::SetSearchText(FString InSearchText) {
	Keywords.Empty();
	FString Keyword;
	while (InSearchText.Len() > 0) {
		if (!InSearchText.Split(TEXT(" "), &Keyword, &InSearchText)) {
			Keyword = InSearchText;
			InSearchText.Empty();
		}
		Keywords.Add(Keyword);
	}
}

bool FFIVSEdTextSearch::Search(FString InSearchableText, int* OutSignificance) {
	int KeywordLength = 0;
	for (const FString& Keyword : Keywords) {
		if (!InSearchableText.Contains(Keyword)) return false;
		KeywordLength += Keyword.Len();
	}
	if (OutSignificance) {
		*OutSignificance = TNumericLimits<int>::Max() - FMath::Abs(InSearchableText.Replace(TEXT(" "), TEXT("")).Len() - KeywordLength);
	}
	return true;
}
