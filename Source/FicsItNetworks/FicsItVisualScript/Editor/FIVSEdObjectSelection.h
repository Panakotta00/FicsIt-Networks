#pragma once

#include "FIVSEdSearchListView.h"
#include "FicsItNetworks/Network/FINNetworkCircuit.h"
#include "FicsItNetworks/Network/FINNetworkUtils.h"

class SFIVSEdObjectSelection : public SCompoundWidget {
	DECLARE_DELEGATE_OneParam(FSelectionChanged, const FFINNetworkTrace&)
	
	SLATE_BEGIN_ARGS(SFIVSEdObjectSelection) {}
	SLATE_EVENT(FSelectionChanged, OnSelectionChanged)
	SLATE_ARGUMENT(FFINNetworkTrace, InitSelection)
	SLATE_END_ARGS()

	TSharedPtr<SBox> WidgetHolder;
	FSelectionChanged OnSelectionChanged;
	TArray<FFINNetworkTrace> Components;
	
public:
	void Construct(const FArguments& InArgs, const TArray<FFINNetworkTrace>& InComponents) {
		OnSelectionChanged = InArgs._OnSelectionChanged;
		Components = InComponents;
		Components.Add(FFINNetworkTrace());
		
		ChildSlot[
			SAssignNew(WidgetHolder, SBox)
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

	void SelectObject(const FFINNetworkTrace& Component) {
		WidgetHolder->SetContent(CreateComponentWidget(Component));
		if (!Component.IsValid()) return;
		OnSelectionChanged.ExecuteIfBound(Component);
	}

	TSharedRef<SWidget> CreateComponentWidget(const FFINNetworkTrace& Component) {
		if (Component.GetUnderlyingPtr())
			return SNew(SHorizontalBox)
			+SHorizontalBox::Slot()[
				SNew(STextBlock)
				.Text(FText::FromString(IFINNetworkComponent::Execute_GetID(*Component).ToString()))
			]
			+SHorizontalBox::Slot()[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor(1,1,1,0.7))
				.Text(FText::FromString(IFINNetworkComponent::Execute_GetNick(*Component)))
			];
		else
			return SNew(STextBlock)
			.Text(FText::FromString(TEXT("(None)")));
	}

	TSharedRef<SWidget> CreateSignalSearch() {
		return SNew(SFIVSEdSearchListView<FFINNetworkTrace>, Components)
		.OnGetSearchableText_Lambda([](FFINNetworkTrace Trace) -> FString {
			if (!Trace.GetUnderlyingPtr()) return TEXT("None");
			return IFINNetworkComponent::Execute_GetID(*Trace).ToString() + TEXT(" ") + IFINNetworkComponent::Execute_GetNick(*Trace);
		})
		.OnGetElementWidget_Lambda([this](FFINNetworkTrace Trace) -> TSharedRef<SWidget> {
			return CreateComponentWidget(Trace);
		})
		.OnCommited_Lambda([this](FFINNetworkTrace Trace) {
			SelectObject(Trace);
			FSlateApplication::Get().DismissAllMenus();
		});
	}
};
