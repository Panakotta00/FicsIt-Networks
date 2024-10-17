#pragma once

#include "CoreMinimal.h"
#include "Slate.h"

class UFIRSignal;

class SFIVSEdSignalSelection : public SCompoundWidget {
	DECLARE_DELEGATE_OneParam(FSelectionChanged, UFIRSignal*)
	
	SLATE_BEGIN_ARGS(SFIVSEdSignalSelection) : _InitSelection(nullptr) {}
	SLATE_EVENT(FSelectionChanged, OnSelectionChanged)
	SLATE_ARGUMENT(UFIRSignal*, InitSelection)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SBox> SignalWidgetHolder;
	FSelectionChanged OnSelectionChanged;

public:
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void SelectObject(UFIRSignal* Signal);

	TSharedRef<SWidget> CreateSmallSignalWidget(UFIRSignal* InSignal);

	TSharedRef<SWidget> CreateSignalSearch();
};
