#include "FIVSEdNodeViewer.h"

void SFIVSEdPinViewer::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	SetPin(InArgs._Pin);
}

SFIVSEdPinViewer::SFIVSEdPinViewer() {}

FReply SFIVSEdPinViewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (!MouseEvent.GetModifierKeys().IsControlDown() && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		TSharedPtr<IMenu> MenuHandle;
		FMenuBuilder MenuBuilder(true, NULL);
		MenuBuilder.AddMenuEntry(
			FText::FromString("Remove Connections"),
            FText(),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([this]() {
				TArray<UFIVSPin*> Pins = Pin->GetConnections();
	            for (UFIVSPin* Pin : Pins) Pin->RemoveConnection(GetPin());
			})));
		
		FSlateApplication::Get().PushMenu(SharedThis(this), *MouseEvent.GetEventPath(), MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FSlateColor SFIVSEdPinViewer::GetPinColor() const {
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

void SFIVSEdPinViewer::SetPin(UFIVSPin* newPin) {
	Pin = newPin;
	PinIconWidget = SNew(SBox)
	.HeightOverride(16)
	.WidthOverride(16)
	.Content()[
		SNew(SImage)
		.Image_Lambda([this]() {
			if (Pin->GetConnections().Num() > 0) {
				return &Style->DataPinConnectedIcon;
			} else {
				return &Style->DataPinIcon;
			}
		})
		.ColorAndOpacity_Raw(this, &SFIVSEdPinViewer::GetPinColor)
	];
	if (Pin->GetName().ToString().Len() == 0) {
		ChildSlot[
			SNew(SBorder)
	        .BorderBackgroundColor_Lambda([this]() {
	            return (IsHovered() && !FSlateApplication::Get().GetModifierKeys().IsControlDown()) ? FLinearColor(FColor::White) : FColor::Transparent;
	        })
	        .Content()[
	            PinIconWidget.ToSharedRef()
	        ]
	    ];
	} else if (Pin->GetPinType() & FIVS_PIN_INPUT) {
		ChildSlot[
			SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]() {
	            return (IsHovered() && !FSlateApplication::Get().GetModifierKeys().IsControlDown()) ? FLinearColor(FColor::White) : FColor::Transparent;
	        })
			.Content()[
	            SNew(SHorizontalBox)
	            +SHorizontalBox::Slot()
				.AutoWidth()
	            .Padding(1)
	            .VAlign(EVerticalAlignment::VAlign_Center)[
	                PinIconWidget.ToSharedRef()
	            ]
	            +SHorizontalBox::Slot()
				.FillWidth(1)
	            .Padding(5)
	            .VAlign(EVerticalAlignment::VAlign_Center)[
	                SNew(STextBlock)
	                .Clipping(EWidgetClipping::Inherit)
	                .Justification(ETextJustify::Left)
	                .Text_Lambda([this]() {
	                    return Pin->GetName();
	                })
	            ]
			]
		];
	} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
		ChildSlot[
            SNew(SBorder)
            .BorderBackgroundColor_Lambda([this]() {
                return (IsHovered() && !FSlateApplication::Get().GetModifierKeys().IsControlDown()) ? FLinearColor(FColor::White) : FColor::Transparent;
            })
            .Content()[SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .FillWidth(1)
                .Padding(5)
                .VAlign(EVerticalAlignment::VAlign_Center)[
                    SNew(STextBlock)
					.Clipping(EWidgetClipping::Inherit)
                    .Justification(ETextJustify::Left)
                    .Text_Lambda([this]() {
                        return Pin->GetName();
                    })
                ]
                +SHorizontalBox::Slot()
				.AutoWidth()
                .Padding(1)
                .VAlign(EVerticalAlignment::VAlign_Center)[
                    PinIconWidget.ToSharedRef()
                ]
			]
		];
	}
}

UFIVSPin* SFIVSEdPinViewer::GetPin() const {
	return Pin;
}

FVector2D SFIVSEdPinViewer::GetConnectionPoint() const {
	if (PinIconWidget.IsValid()) return PinIconWidget->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0.5, 0.5));
	return FVector2D();
}

void SFIVSEdNodeViewer::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	SetNode(InArgs._Node.Get());
}

SFIVSEdNodeViewer::SFIVSEdNodeViewer() {}

FReply SFIVSEdNodeViewer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	PinUnderMouse = nullptr;
	for (TSharedRef<SFIVSEdPinViewer> Pin : PinWidgets) {
		if (Pin->GetCachedGeometry().IsUnderLocation(MouseEvent.GetScreenSpacePosition())) {
			PinUnderMouse = Pin->GetPin();
			break;
		}
	}
	
	return FReply::Unhandled();
}

void SFIVSEdNodeViewer::SetNode(UFIVSNode* newNode) {
	if (InputPinBox) InputPinBox->ClearChildren();
	if (OutputPinBox) OutputPinBox->ClearChildren();
	PinWidgets.Empty();
	PinToWidget.Empty();

	Node = newNode;

	if (Node && Node->IsA<UFIVSFuncNode>()) {
		ChildSlot[
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
                                return FText::FromString(Cast<UFIVSFuncNode>(Node)->GetNodeName());
                            })
                        ]
                    ]
                    +SGridPanel::Slot(0, 1)[
                        SAssignNew(InputPinBox, SVerticalBox)
                    ]
                    +SGridPanel::Slot(1, 1)[
                        SNew(SSpacer).Size(FVector2D(20, 20))
                    ]
                    +SGridPanel::Slot(2, 1)[
                        SAssignNew(OutputPinBox, SVerticalBox)
                    ]
                ]
            ]
        ];

		for (UFIVSPin* Pin : newNode->GetNodePins()) {
			if (Pin->GetPinType() & FIVS_PIN_INPUT) {
				TSharedRef<SFIVSEdPinViewer> PinWidget = SNew(SFIVSEdPinViewer)
				.Style(Style)
                .Pin(Pin);
				PinWidgets.Add(PinWidget);
				PinToWidget.Add(Pin, PinWidget);
				InputPinBox->AddSlot()[
                    PinWidget
                ];
			} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
				TSharedRef<SFIVSEdPinViewer> PinWidget = SNew(SFIVSEdPinViewer)
				.Style(Style)
                .Pin(Pin);
				PinWidgets.Add(PinWidget);
				PinToWidget.Add(Pin, PinWidget);
				OutputPinBox->AddSlot()[
                    PinWidget
                ];
			}
		}
	} else if (Node->IsA<UFIVSRerouteNode>()) {
		TSharedPtr<SFIVSEdPinViewer> PinWidget;
		UFIVSPin* Pin = Node->GetNodePins()[0];
		ChildSlot[
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
					SAssignNew(PinWidget, SFIVSEdPinViewer)
					.Style(Style)
					.Pin(Pin)
                ]
            ]
        ];
		PinWidgets.Add(PinWidget.ToSharedRef());
		PinToWidget.Add(Pin, PinWidget.ToSharedRef());
	}
}

UFIVSNode* SFIVSEdNodeViewer::GetNode() const {
	return Node;
}

FVector2D SFIVSEdNodeViewer::GetPosition() const {
	return Node->Pos;
}

const TArray<TSharedRef<SFIVSEdPinViewer>>& SFIVSEdNodeViewer::GetPinWidgets() {
	return PinWidgets;
}

UFIVSPin* SFIVSEdNodeViewer::GetPinUnderMouse() {
	return PinUnderMouse;
}

TSharedRef<SFIVSEdPinViewer> SFIVSEdNodeViewer::GetPinWidget(UFIVSPin* Pin) {
	return PinToWidget[Pin];
}
