#pragma once

#include "CoreMinimal.h"

class FICSITVISUALSCRIPT_API FFIVSEdTextSearch {
private:
	TArray<FString> Keywords;
	
public:
	void SetSearchText(FString InSearchText);

	bool Search(FString InSearchableText, int* OutSignificance);
};
