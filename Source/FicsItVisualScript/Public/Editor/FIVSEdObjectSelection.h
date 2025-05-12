#pragma once

#include "CoreMinimal.h"
#include "CoreStyle.h"
#include "FIRTrace.h"
#include "SBox.h"
#include "SCompoundWidget.h"
#include "SlateImageBrush.h"

class SFIVSEdObjectSelection : public SCompoundWidget {
	DECLARE_DELEGATE_OneParam(FSelectionChanged, const FFIRTrace&)
	
	SLATE_BEGIN_ARGS(SFIVSEdObjectSelection) :
		_PrimaryFont(FCoreStyle::GetDefaultFontStyle("Regular", FCoreStyle::RegularTextSize)),
		_SecondaryFont(FCoreStyle::GetDefaultFontStyle("Regular", FCoreStyle::SmallTextSize)) {}
	SLATE_EVENT(FSelectionChanged, OnSelectionChanged)
	SLATE_ARGUMENT(FFIRTrace, InitSelection)
	SLATE_ATTRIBUTE(FSlateFontInfo, PrimaryFont)
	SLATE_ATTRIBUTE(FSlateFontInfo, SecondaryFont)
	SLATE_END_ARGS()

	TSharedPtr<SBox> WidgetHolder;
	FSelectionChanged OnSelectionChanged;
	TArray<FFIRTrace> Components;
	TAttribute<FSlateFontInfo> PrimaryFont;
	TAttribute<FSlateFontInfo> SecondaryFont;
	
public:
	void Construct(const FArguments& InArgs, const TArray<FFIRTrace>& InComponents);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void SelectObject(const FFIRTrace& Component);

	TSharedRef<SWidget> CreateComponentWidget(const FFIRTrace& Component);

	TSharedRef<SWidget> CreateLargeComponentWidget(const FFIRTrace& Component);

	TSharedRef<SWidget> CreateSignalSearch();
};

class SFIVSEdComponentWidget : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFIVSEdComponentWidget) :
		_PrimaryFont(FCoreStyle::GetDefaultFontStyle("Regular", FCoreStyle::RegularTextSize)),
		_SecondaryFont(FCoreStyle::GetDefaultFontStyle("Regular", FCoreStyle::SmallTextSize)) {}
	SLATE_ATTRIBUTE(FSlateFontInfo, PrimaryFont)
	SLATE_ATTRIBUTE(FSlateFontInfo, SecondaryFont)
	SLATE_END_ARGS()

private:
	TAttribute<FSlateFontInfo> PrimaryFont;
	TAttribute<FSlateFontInfo> SecondaryFont;
	FSlateBrush Brush;

public:
	void Construct(const FArguments& InArgs, const FFIRTrace& Component);
};
