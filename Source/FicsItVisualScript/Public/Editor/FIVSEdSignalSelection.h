#pragma once

#include "CoreMinimal.h"
#include "Slate.h"

class UFINSignal;

class SFIVSEdSignalSelection : public SCompoundWidget {
	DECLARE_DELEGATE_OneParam(FSelectionChanged, UFINSignal*)
	
	SLATE_BEGIN_ARGS(SFIVSEdSignalSelection) : _InitSelection(nullptr) {}
	SLATE_EVENT(FSelectionChanged, OnSelectionChanged)
	SLATE_ARGUMENT(UFINSignal*, InitSelection)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SBox> SignalWidgetHolder;
	FSelectionChanged OnSelectionChanged;

public:
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	void SelectObject(UFINSignal* Signal);

	TSharedRef<SWidget> CreateSmallSignalWidget(UFINSignal* InSignal);

	TSharedRef<SWidget> CreateSignalSearch();
};
