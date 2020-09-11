#include "FINScriptNodeViewer.h"

void SFINScriptPinViewer::Construct(const FArguments& InArgs) {
	SetPin(InArgs._Pin.Get().ToSharedRef());
}

SFINScriptPinViewer::SFINScriptPinViewer() : Children(this) {}

FVector2D SFINScriptPinViewer::ComputeDesiredSize(float) const {
	if (Children.Num() > 0)return Children[0]->GetDesiredSize();
	return FVector2D(10, 10);
}

FChildren* SFINScriptPinViewer::GetChildren() {
	return &Children;
}

void SFINScriptPinViewer::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	if (Children.Num() > 0) ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Children[0], FVector2D(), Children[0]->GetDesiredSize(), 1));
}

FReply SFINScriptPinViewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (!MouseEvent.GetModifierKeys().IsControlDown() && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		TSharedPtr<IMenu> MenuHandle;
		FMenuBuilder MenuBuilder(true, NULL);
		MenuBuilder.AddMenuEntry(
			FText::FromString("Remove Connections"),
            FText(),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([this]() {
				TArray<TSharedPtr<FFINScriptPin>> Pins = Pin->GetConnections();
	            for (const TSharedPtr<FFINScriptPin>& Pin : Pins) Pin->RemoveConnection(GetPin());
			})));
		
		FSlateApplication::Get().PushMenu(SharedThis(this), *MouseEvent.GetEventPath(), MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FSlateColor SFINScriptPinViewer::GetPinColor() const {
	if (Pin->GetPinType() & FIVS_PIN_DATA) {
		switch (Pin->GetPinDataType()) {
		case FIN_BOOL:		return FLinearColor(FColor::FromHex("FF0000"));
		case FIN_CLASS:		return FLinearColor(FColor::FromHex("AA00AA"));
		case FIN_FLOAT:		return FLinearColor(FColor::FromHex("00FF00"));
		case FIN_INT:		return FLinearColor(FColor::FromHex("77FF77"));
		case FIN_OBJ:		return FLinearColor(FColor::FromHex("0000FF"));
		case FIN_STR:		return FLinearColor(FColor::FromHex("FFAA00"));
		case FIN_STRUCT:	return FLinearColor(FColor::FromHex("000077"));
		case FIN_TRACE:		return FLinearColor(FColor::FromHex("FF4400"));
		default:
			break;
		};
	}
	return FLinearColor(FColor::White);
}

void SFINScriptPinViewer::SetPin(const TSharedPtr<FFINScriptPin>& newPin) {
	Children.Empty();
	Pin = newPin;
	PinIconWidget = SNew(SImage).ColorAndOpacity_Raw(this, &SFINScriptPinViewer::GetPinColor);
	if (Pin->Name.Len() == 0) {
		Children.Add(SNew(SBorder)
        .BorderBackgroundColor_Lambda([this]() {
            return (IsHovered() && !FSlateApplication::Get().GetModifierKeys().IsControlDown()) ? FLinearColor(FColor::White) : FColor::Transparent;
        })
        .Content()[
            PinIconWidget.ToSharedRef()
        ]);
	} else if (Pin->GetPinType() & FIVS_PIN_INPUT) {
		Children.Add(SNew(SBorder)
		.BorderBackgroundColor_Lambda([this]() {
            return (IsHovered() && !FSlateApplication::Get().GetModifierKeys().IsControlDown()) ? FLinearColor(FColor::White) : FColor::Transparent;
        })
		.Content()[
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot().Padding(1).VAlign(EVerticalAlignment::VAlign_Center)[
                PinIconWidget.ToSharedRef()
            ]
            +SHorizontalBox::Slot().Padding(5).VAlign(EVerticalAlignment::VAlign_Center)[
                SNew(STextBlock)
                .Clipping(EWidgetClipping::Inherit)
                .Justification(ETextJustify::Left)
                .Text_Lambda([this]() {
                    return FText::FromString(Pin->Name);
                })
            ]
		]);
	} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
		Children.Add(
            SNew(SBorder)
            .BorderBackgroundColor_Lambda([this]() {
                return (IsHovered() && !FSlateApplication::Get().GetModifierKeys().IsControlDown()) ? FLinearColor(FColor::White) : FColor::Transparent;
            })
            .Content()[SNew(SHorizontalBox)
                +SHorizontalBox::Slot().Padding(5).VAlign(EVerticalAlignment::VAlign_Center)[
                    SNew(STextBlock)
					.Clipping(EWidgetClipping::Inherit)
                    .Justification(ETextJustify::Left)
                    .Text_Lambda([this]() {
                        return FText::FromString(Pin->Name);
                    })
                ]
                +SHorizontalBox::Slot().Padding(1).VAlign(EVerticalAlignment::VAlign_Center)[
                    PinIconWidget.ToSharedRef()
                ]
			]);
	}
}

TSharedPtr<FFINScriptPin> SFINScriptPinViewer::GetPin() const {
	return Pin.ToSharedRef();
}

FVector2D SFINScriptPinViewer::GetConnectionPoint() const {
	if (PinIconWidget.IsValid()) return PinIconWidget->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0.5, 0.5));
	return FVector2D();
}

void SFINScriptNodeViewer::Construct(const FArguments& InArgs) {
	SetNode(InArgs._Node.Get());
}

SFINScriptNodeViewer::SFINScriptNodeViewer() : Children(this) {}

FVector2D SFINScriptNodeViewer::ComputeDesiredSize(float) const {
	return Children[0]->GetDesiredSize();
}

FChildren* SFINScriptNodeViewer::GetChildren() {
	return &Children;
}

void SFINScriptNodeViewer::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Children[0], FVector2D(), Children[0]->GetDesiredSize(), 1));
}

FReply SFINScriptNodeViewer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	PinUnderMouse = nullptr;
	for (TSharedRef<SFINScriptPinViewer> Pin : PinWidgets) {
		if (Pin->GetCachedGeometry().IsUnderLocation(MouseEvent.GetScreenSpacePosition())) {
			PinUnderMouse = Pin->GetPin();
		}
	}
	
	return FReply::Unhandled();
}

void SFINScriptNodeViewer::SetNode(UFINScriptNode* newNode) {
	if (InputPinBox) InputPinBox->ClearChildren();
	if (OutputPinBox) OutputPinBox->ClearChildren();
	PinWidgets.Empty();
	PinToWidget.Empty();

	Node = newNode;

	if (Node && Node->IsA<UFINScriptFuncNode>()) {
		Children.Add(
            SNew(SBorder)
            .Padding(1)
            .BorderImage_Lambda([this]() {
                return bSelected ? &OutlineBrush : nullptr;
            })
            .Content()[
                SNew(SBorder)
                .BorderImage(&NodeBrush)
                .Padding(0)
                .Content()
                [
                    SNew(SGridPanel)
                    +SGridPanel::Slot(0, 0).ColumnSpan(3)[
                        SNew(SBorder)
                        .BorderImage(&HeaderBrush)
                        .Padding(1)
                        .Content()
                        [
                            SNew(STextBlock)
                            .Text_Lambda([this]() {
                                return FText::FromString(Cast<UFINScriptFuncNode>(Node)->GetNodeName());
                            })
                        ]
                    ]
                    +SGridPanel::Slot(0, 1)[
                        (InputPinBox = SNew(SVerticalBox)).ToSharedRef()
                    ]
                    +SGridPanel::Slot(1, 1)[
                        SNew(SSpacer).Size(FVector2D(20, 20))
                    ]
                    +SGridPanel::Slot(2, 1)[
                        (OutputPinBox = SNew(SVerticalBox)).ToSharedRef()
                    ]
                ]
            ]);

		for (const TSharedRef<FFINScriptPin>& Pin : newNode->GetNodePins()) {
			if (Pin->GetPinType() & FIVS_PIN_INPUT) {
				TSharedRef<SFINScriptPinViewer> PinWidget = SNew(SFINScriptPinViewer)
                    .Pin(Pin);
				PinWidgets.Add(PinWidget);
				PinToWidget.Add(Pin, PinWidget);
				InputPinBox->AddSlot()[
                    PinWidget
                ];
			} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
				TSharedRef<SFINScriptPinViewer> PinWidget = SNew(SFINScriptPinViewer)
                    .Pin(Pin);
				PinWidgets.Add(PinWidget);
				PinToWidget.Add(Pin, PinWidget);
				OutputPinBox->AddSlot()[
                    PinWidget
                ];
			}
		}
	} else if (Node->IsA<UFINScriptRerouteNode>()) {
		TSharedPtr<SFINScriptPinViewer> PinWidget;
		TSharedRef<FFINScriptPin> Pin = Node->GetNodePins()[0];
		Children.Add(
            SNew(SBorder)
            .Padding(1)
            .BorderImage_Lambda([this]() {
                return bSelected ? &OutlineBrush : nullptr;
            })
            .Content()[
                SNew(SBorder)
                .BorderImage(&NodeBrush)
                .Padding(5)
                .Content()[
					(PinWidget = SNew(SFINScriptPinViewer)
					.Pin(Pin)).ToSharedRef()
                ]
            ]);
		PinWidgets.Add(PinWidget.ToSharedRef());
		PinToWidget.Add(Pin, PinWidget.ToSharedRef());
	}
}

UFINScriptNode* SFINScriptNodeViewer::GetNode() const {
	return Node;
}

FVector2D SFINScriptNodeViewer::GetPosition() const {
	return Node->Pos;
}

const TArray<TSharedRef<SFINScriptPinViewer>>& SFINScriptNodeViewer::GetPinWidgets() {
	return PinWidgets;
}

TSharedPtr<FFINScriptPin> SFINScriptNodeViewer::GetPinUnderMouse() {
	return PinUnderMouse;
}

TSharedRef<SFINScriptPinViewer> SFINScriptNodeViewer::GetPinWidget(const TSharedPtr<FFINScriptPin> Pin) {
	return PinToWidget[Pin];
}
