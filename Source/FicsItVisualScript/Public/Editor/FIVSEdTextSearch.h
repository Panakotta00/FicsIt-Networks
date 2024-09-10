#pragma once

class FFIVSEdTextSearch {
private:
	TArray<FString> Keywords;
	
public:
	void SetSearchText(FString InSearchText);

	bool Search(FString InSearchableText, int* OutSignificance);
};
