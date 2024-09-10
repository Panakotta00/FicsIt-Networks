#pragma once

#include "CoreMinimal.h"
#include "Network/FINNetworkTrace.h"

class SFIVSEdObjectSelection : public SCompoundWidget {
	DECLARE_DELEGATE_OneParam(FSelectionChanged, const FFINNetworkTrace&)
	
	SLATE_BEGIN_ARGS(SFIVSEdObjectSelection) :
		_PrimaryFont(FCoreStyle::GetDefaultFontStyle("Regular", FCoreStyle::RegularTextSize)),
		_SecondaryFont(FCoreStyle::GetDefaultFontStyle("Regular", FCoreStyle::SmallTextSize)) {}
	SLATE_EVENT(FSelectionChanged, OnSelectionChanged)
	SLATE_ARGUMENT(FFINNetworkTrace, InitSelection)
	SLATE_ATTRIBUTE(FSlateFontInfo, PrimaryFont)
	SLATE_ATTRIBUTE(FSlateFontInfo, SecondaryFont)
	SLATE_END_ARGS()

	TSharedPtr<SBox> WidgetHolder;
	FSelectionChanged OnSelectionChanged;
	TArray<FFINNetworkTrace> Components;
	TAttribute<FSlateFontInfo> PrimaryFont;
	TAttribute<FSlateFontInfo> SecondaryFont;
	
public:
	void Construct(const FArguments& InArgs, const TArray<FFINNetworkTrace>& InComponents);

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void SelectObject(const FFINNetworkTrace& Component);

	TSharedRef<SWidget> CreateComponentWidget(const FFINNetworkTrace& Component);

	TSharedRef<SWidget> CreateLargeComponentWidget(const FFINNetworkTrace& Component);

	TSharedRef<SWidget> CreateSignalSearch();
};
