#pragma once

#include "FINReflectionUIContext.h"
#include "FINReflectionUIStyle.h"
#include "SlateBasics.h"

class SFINReflectionEntryListViewer : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFINReflectionEntryListViewer) :
        _Style(&FFINReflectionUIStyleStruct::GetDefault()) {}
		SLATE_ATTRIBUTE(const FFINReflectionUIStyleStruct*, Style)
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs, TArray<TSharedPtr<FFINReflectionUIEntry>>* InSource, FFINReflectionUIContext* InContext);
private:
	FFINReflectionUIContext* Context = nullptr;
	TAttribute<const FFINReflectionUIStyleStruct*> Style;
	TArray<TSharedPtr<FFINReflectionUIEntry>>* Source = nullptr;
};

