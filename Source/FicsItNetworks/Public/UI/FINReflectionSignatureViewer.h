#pragma once

#include "FINReflectionUIContext.h"
#include "FINReflectionUIStyle.h"

class SFINReflectionSignatureViewer : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFINReflectionSignatureViewer) :
        _Style(&FFINReflectionUIStyleStruct::GetDefault()) {}
		SLATE_ATTRIBUTE(const FFINReflectionUIStyleStruct*, Style)
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs, TArray<UFINProperty*> InSource, FFINReflectionUIContext* InContext);
private:
	FFINReflectionUIContext* Context = nullptr;
	TAttribute<const FFINReflectionUIStyleStruct*> Style;
	TArray<TSharedPtr<UFINProperty*>> Source;
};

