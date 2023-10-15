#pragma once

#include "FIVSEdSearchListView.h"
#include "FicsItNetworks/Reflection/FINClass.h"
#include "FicsItNetworks/Reflection/FINReflection.h"
#include "FicsItNetworks/Reflection/FINSignal.h"

class SFIVSEdSignalSelection : public SCompoundWidget {
	DECLARE_DELEGATE_OneParam(FSelectionChanged, UFINSignal*)
	
	SLATE_BEGIN_ARGS(SFIVSEdSignalSelection) : _InitSelection(nullptr) {}
	SLATE_EVENT(FSelectionChanged, OnSelectionChanged)
	SLATE_ARGUMENT(UFINSignal*, InitSelection)
	SLATE_END_ARGS()

	TSharedPtr<SBox> SignalWidgetHolder;
	FSelectionChanged OnSelectionChanged;
	
public:
	void Construct(const FArguments& InArgs) {
		OnSelectionChanged = InArgs._OnSelectionChanged;
		
		ChildSlot[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("Border"))
			.Content()[
				SAssignNew(SignalWidgetHolder, SBox)
			]
		];

		SelectObject(InArgs._InitSelection);
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override {
		TSharedRef<SWidget> MenuWidget = SNew(SBox)
		.WidthOverride(MyGeometry.GetAbsoluteSize().X)
		.Content()[
			CreateSignalSearch()
		];
		TSharedPtr<IMenu> Menu = FSlateApplication::Get().PushMenu(SharedThis(this), FWidgetPath(), MenuWidget, MyGeometry.AbsolutePosition, FPopupTransitionEffect::None);
		return FReply::Handled();
	}

	void SelectObject(UFINSignal* Signal) {
		SignalWidgetHolder->SetContent(CreateSmallSignalWidget(Signal));
		OnSelectionChanged.ExecuteIfBound(Signal);
	}

	TSharedRef<SWidget> CreateSmallSignalWidget(UFINSignal* InSignal) {
		if (InSignal)
			return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()[
				SNew(STextBlock)
				.Text(InSignal->GetDisplayName())
			];
		else
			return SNew(STextBlock)
			.Text(FText::FromString(TEXT("(None)")));
	}

	TSharedRef<SWidget> CreateSignalSearch() {
		TArray<UFINSignal*> Elements;
		for (TPair<UClass*, UFINClass*> Class : FFINReflection::Get()->GetClasses()) {
			for (UFINSignal* Signal : Class.Value->GetSignals(false)) {
				Elements.Add(Signal);
			}
		}
		Elements.Add(nullptr);

		return SNew(SFIVSEdSearchListView<UFINSignal*>, Elements)
		.OnGetSearchableText_Lambda([](UFINSignal* InSignal) -> FString {
			if (!InSignal) return TEXT("None");
			return InSignal->GetDisplayName().ToString() + TEXT(" ") + Cast<UFINClass>(InSignal->GetOuter())->GetDisplayName().ToString();
		})
		.OnGetElementWidget_Lambda([this](UFINSignal* InSignal) {
			return CreateSmallSignalWidget(InSignal);
		})
		.OnCommited_Lambda([this](UFINSignal* InSignal) {
			SelectObject(InSignal);
			FSlateApplication::Get().DismissAllMenus();
		});
	}
};
