#include "Editor/FIVSEdObjectSelection.h"

#include "Network/FINNetworkComponent.h"
#include "Editor/FIVSEdSearchListView.h"

void SFIVSEdObjectSelection::Construct(const FArguments& InArgs, const TArray<FFINNetworkTrace>& InComponents) {
	OnSelectionChanged = InArgs._OnSelectionChanged;
	PrimaryFont = InArgs._PrimaryFont;
	SecondaryFont = InArgs._SecondaryFont;

	Components = InComponents;
	Components.Add(FFINNetworkTrace());

	ChildSlot[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("Border"))
		.Content()[
			SAssignNew(WidgetHolder, SBox)
		]
	];

	SelectObject(InArgs._InitSelection);
}

FReply SFIVSEdObjectSelection::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	TSharedRef<SWidget> MenuWidget = SNew(SBox)
		.WidthOverride(MyGeometry.GetAbsoluteSize().X)
		.Content()[
			CreateSignalSearch()
		];
	TSharedPtr<IMenu> Menu = FSlateApplication::Get().PushMenu(AsShared(), FWidgetPath(), MenuWidget, (FVector2D)MyGeometry.AbsolutePosition, FPopupTransitionEffect(FPopupTransitionEffect::None));
	return FReply::Handled();
}

void SFIVSEdObjectSelection::SelectObject(const FFINNetworkTrace& Component) {
	WidgetHolder->SetContent(CreateComponentWidget(Component));
	if (!Component.IsValid()) return;
	bool _ = OnSelectionChanged.ExecuteIfBound(Component);
}

TSharedRef<SWidget> SFIVSEdObjectSelection::CreateComponentWidget(const FFINNetworkTrace& Component) {
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

TSharedRef<SWidget> SFIVSEdObjectSelection::CreateLargeComponentWidget(const FFINNetworkTrace& Component) {
	if (Component.GetUnderlyingPtr()) {
		TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
		FString Nick = IFINNetworkComponent::Execute_GetNick(*Component);
		FString ID = IFINNetworkComponent::Execute_GetID(*Component).ToString();
		if (Nick.Len() > 0) {
			Box->AddSlot()[
				SNew(STextBlock)
				.Text(FText::FromString(Nick))
				.Font(PrimaryFont)
			];
			Box->AddSlot()[
				SNew(STextBlock)
				.Text(FText::FromString(ID))
			];
		} else {
			Box->AddSlot()[
				SNew(STextBlock)
				.Text(FText::FromString(ID))
			];
		}
		return Box;
	} else {
		return SNew(STextBlock)
			.Text(FText::FromString(TEXT("(None)")));
	}
}

TSharedRef<SWidget> SFIVSEdObjectSelection::CreateSignalSearch() {
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
