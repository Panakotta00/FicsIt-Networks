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
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual void OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) override;

private:
	FFINReflectionUIContext* Context = nullptr;
	TAttribute<const FFINReflectionUIStyleStruct*> Style;
	TArray<TSharedPtr<FFINReflectionUIEntry>>* Source = nullptr;

	TSharedPtr<SListView<TSharedPtr<FFINReflectionUIEntry>>> List;

	void UpdateList(FFINReflectionUIEntry* Entry);
};

