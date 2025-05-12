#include "Editor/FIVSEdObjectSelection.h"

#include "FGBuildable.h"
#include "FINNetworkComponent.h"
#include "FINNetworkUtils.h"
#include "Editor/FIVSEdSearchListView.h"

void SFIVSEdObjectSelection::Construct(const FArguments& InArgs, const TArray<FFIRTrace>& InComponents) {
	OnSelectionChanged = InArgs._OnSelectionChanged;
	PrimaryFont = InArgs._PrimaryFont;
	SecondaryFont = InArgs._SecondaryFont;

	Components = InComponents;
	Components.Add(FFIRTrace());

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

void SFIVSEdObjectSelection::SelectObject(const FFIRTrace& Component) {
	WidgetHolder->SetContent(CreateComponentWidget(Component));
	if (!Component.IsValid()) return;
	bool _ = OnSelectionChanged.ExecuteIfBound(Component);
}

TSharedRef<SWidget> SFIVSEdObjectSelection::CreateComponentWidget(const FFIRTrace& Component) {
	if (Component.GetUnderlyingPtr()) {
		return SNew(SFIVSEdComponentWidget, Component);
	} else {
		return SNew(STextBlock)
				.Text(FText::FromString(TEXT("(None)")));
	}
}

TSharedRef<SWidget> SFIVSEdObjectSelection::CreateLargeComponentWidget(const FFIRTrace& Component) {
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
	return SNew(SFIVSEdSearchListView<FFIRTrace>, Components)
		.OnGetSearchableText_Lambda([](FFIRTrace Trace) -> FString {
			if (!Trace.GetUnderlyingPtr()) return TEXT("None");
			return IFINNetworkComponent::Execute_GetID(*Trace).ToString() + TEXT(" ") + IFINNetworkComponent::Execute_GetNick(*Trace);
		})
		.OnGetElementWidget_Lambda([this](FFIRTrace Trace) -> TSharedRef<SWidget> {
			return CreateComponentWidget(Trace);
		})
		.OnCommited_Lambda([this](FFIRTrace Trace) {
			SelectObject(Trace);
			FSlateApplication::Get().DismissAllMenus();
		});
}

void SFIVSEdComponentWidget::Construct(const FArguments& InArgs, const FFIRTrace& Component) {
	UTexture2D* icon = nullptr;
	if (auto buildable = Cast<AFGBuildable>(UFINNetworkUtils::RedirectIfPossible(Component).Get())) {
		icon = UFGItemDescriptor::GetSmallIcon(buildable->GetBuiltWithDescriptor());
	}
	Brush = static_cast<FSlateBrush>(FSlateImageBrush(icon, FVector2D(32, 32)));

	ChildSlot[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot().AutoWidth()[
			SNew(SImage)
			.Image(&Brush)
		]
		+SHorizontalBox::Slot()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()[
				SNew(STextBlock)
				.Text(FText::FromString(IFINNetworkComponent::Execute_GetID(*Component).ToString()))
			]
			+SVerticalBox::Slot()[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor(1,1,1,0.7))
				.Text(FText::FromString(IFINNetworkComponent::Execute_GetNick(*Component)))
			]
		]
	];
}
