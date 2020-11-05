#pragma once

#include "FINReflectionUIStyle.h"
#include "SlateBasics.h"
#include "FINReflectionUIContext.h"
#include "FINReflectionUI.generated.h"

class SFINReflectionClassHirachyViewer : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFINReflectionClassHirachyViewer) :
        _Style(&FFINReflectionUIStyleStruct::GetDefault()) {}
		SLATE_ATTRIBUTE(const FFINReflectionUIStyleStruct*, Style)
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs, const TSharedPtr<FFINReflectionUIClass>& SearchClass, FFINReflectionUIContext* Context);
private:
	FFINReflectionUIContext* Context = nullptr;
	TAttribute<const FFINReflectionUIStyleStruct*> Style;
	TSharedPtr<FFINReflectionUIClass> SearchClass;
	TArray<TSharedPtr<FFINReflectionUIClass>> ClassSource;
};
