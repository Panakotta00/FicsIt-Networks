#include "Editor/FIVSEdSignalSelection.h"

#include "Reflection/FINClass.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINSignal.h"
#include "Editor/FIVSEdSearchListView.h"

void SFIVSEdSignalSelection::Construct(const FArguments& InArgs) {
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

FReply SFIVSEdSignalSelection::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	TSharedRef<SWidget> MenuWidget = SNew(SBox)
		.WidthOverride(MyGeometry.GetAbsoluteSize().X)
		.Content()[
			CreateSignalSearch()
		];
	TSharedPtr<IMenu> Menu = FSlateApplication::Get().PushMenu(SharedThis(this), FWidgetPath(), MenuWidget, (FVector2D)MyGeometry.AbsolutePosition, FPopupTransitionEffect::None);
	return FReply::Handled();
}

void SFIVSEdSignalSelection::SelectObject(UFINSignal* Signal) {
	SignalWidgetHolder->SetContent(CreateSmallSignalWidget(Signal));
	OnSelectionChanged.ExecuteIfBound(Signal);
}

TSharedRef<SWidget> SFIVSEdSignalSelection::CreateSmallSignalWidget(UFINSignal* InSignal) {
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

TSharedRef<SWidget> SFIVSEdSignalSelection::CreateSignalSearch() {
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
