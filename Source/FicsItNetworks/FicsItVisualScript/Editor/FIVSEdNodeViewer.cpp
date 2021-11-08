#include "FIVSEdNodeViewer.h"

#include "FIVSEdGraphViewer.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSScriptNode.h"
#include "Widgets/Input/SNumericEntryBox.h"

void SFIVSEdPinViewer::Construct(const FArguments& InArgs, SFIVSEdNodeViewer* InNodeViewer, UFIVSPin* InPin) {
	Style = InArgs._Style;
	NodeViewer = InNodeViewer;
	Pin = InPin;
	
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
	if (Pin->GetDisplayName().ToString().Len() == 0) {
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
		TSharedRef<SBox> LiteralBox = SNew(SBox)
		.Visibility_Lambda([this]() {
			return Pin->GetConnections().Num() < 1 ? EVisibility::Visible : EVisibility::Collapsed;
		});
		SGridPanel::FSlot* Slot1;
		SGridPanel::FSlot* Slot2;
		ChildSlot[
			SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]() {
	            return (IsHovered() && !FSlateApplication::Get().GetModifierKeys().IsControlDown()) ? FLinearColor(FColor::White) : FColor::Transparent;
	        })
			.Content()[
	            SNew(SGridPanel)
	            .FillColumn(1, 1)
	            +SGridPanel::Slot(0, 0)
	            .Padding(1)
	            .VAlign(VAlign_Center)
	            .HAlign(HAlign_Center)[
	                PinIconWidget.ToSharedRef()
	            ]
	            +SGridPanel::Slot(1, 0)
				.Padding(5)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				.Expose(Slot1)
	            +SGridPanel::Slot(1, 1)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Fill)
                .Expose(Slot2)
            ]
		];
		if (InArgs._ShowName) {
			(*Slot1)[
				SNew(STextBlock)
				.Clipping(EWidgetClipping::Inherit)
				.Text_Lambda([this]() {
					return Pin->GetDisplayName();
				})
			];
			(*Slot2)[
				LiteralBox
			];
		} else {
			(*Slot1)[
				LiteralBox
			];
		}
		
		TSharedPtr<SWidget> LiteralWidget;
		switch (Pin->GetPinDataType().GetType()) {
		case FIN_BOOL:
			LiteralBox->SetContent(SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState InState) {
				if (InState == ECheckBoxState::Checked) {
					Pin->SetLiteral(true);
				} else {
					Pin->SetLiteral(false);
				}
			})
			.IsChecked_Lambda([this]() {
				return Pin->GetLiteral().GetBool() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
			}));
			break;
		case FIN_INT:
			LiteralWidget = SNew(SNumericEntryBox<int>)
			.OnValueChanged_Lambda([this](int Value) {
				Pin->SetLiteral((FINInt)Value);
			})
			.Value_Lambda([this]() {
				return Pin->GetLiteral().GetInt();
			});
			break;
		case FIN_FLOAT:
			LiteralWidget = SNew(SNumericEntryBox<double>)
			.OnValueChanged_Lambda([this](double Value) {
				Pin->SetLiteral((FINFloat)Value);
			})
			.Value_Lambda([this]() {
				return Pin->GetLiteral().GetFloat();
			});
			break;
		case FIN_STR:
			LiteralWidget = SNew(SEditableTextBox)
			.OnTextCommitted_Lambda([this](FText Value, ETextCommit::Type) {
				Pin->SetLiteral(Value.ToString());
			})
			.Text_Lambda([this]() {
				return FText::FromString(Pin->GetLiteral().GetString());
			})
			.MinDesiredWidth(100);
			break;
		case FIN_OBJ:
			break;
		case FIN_CLASS:
			break;
		case FIN_TRACE:
			break;
		case FIN_STRUCT:
			break;
		case FIN_ARRAY:
			break;
		case FIN_ANY:
			break;
		default: ;
		}
		if (LiteralWidget) {
			LiteralBox->SetPadding(5);
			LiteralBox->SetContent(LiteralWidget.ToSharedRef());
		}
	} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
		SHorizontalBox::FSlot* Slot;
		ChildSlot[
            SNew(SBorder)
            .BorderBackgroundColor_Lambda([this]() {
                return (IsHovered() && !FSlateApplication::Get().GetModifierKeys().IsControlDown()) ? FLinearColor(FColor::White) : FColor::Transparent;
            })
            .Content()[SNew(SHorizontalBox)
                +SHorizontalBox::Slot()
                .FillWidth(1)
                .Padding(5)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Right)
                .Expose(Slot)
                +SHorizontalBox::Slot()
				.AutoWidth()
                .Padding(1)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)[
                    PinIconWidget.ToSharedRef()
                ]
			]
		];
		if (InArgs._ShowName) (*Slot)[
			SNew(STextBlock)
			.Clipping(EWidgetClipping::Inherit)
			.Text_Lambda([this]() {
				return Pin->GetDisplayName();
			})
		];
	}
}

SFIVSEdPinViewer::SFIVSEdPinViewer() {}

FReply SFIVSEdPinViewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (!MouseEvent.GetModifierKeys().IsControlDown() && MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
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
	return FReply::Handled().DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
}

FReply SFIVSEdPinViewer::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	TSharedRef<FFIVSEdPinConnectDragDrop> DragDropOperation = MakeShared<FFIVSEdPinConnectDragDrop>(SharedThis(this));
	return FReply::Handled().BeginDragDrop(DragDropOperation);
}

FReply SFIVSEdPinViewer::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
	TSharedPtr<FFIVSEdPinConnectDragDrop> Operator = DragDropEvent.GetOperationAs<FFIVSEdPinConnectDragDrop>();
	if (Operator.IsValid()) {
		Operator->GetPinViewer()->GetPin()->AddConnection(GetPin());
		return FReply::Handled().EndDragDrop();
	}
	return SCompoundWidget::OnDrop(MyGeometry, DragDropEvent);
}

FSlateColor SFIVSEdPinViewer::GetPinColor() const {
	if (Pin->GetPinType() & FIVS_PIN_DATA) {
		switch (Pin->GetPinDataType().GetType()) {
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

UFIVSPin* SFIVSEdPinViewer::GetPin() const {
	return Pin;
}

FVector2D SFIVSEdPinViewer::GetConnectionPoint() const {
	if (PinIconWidget.IsValid()) return PinIconWidget->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0.5, 0.5));
	return FVector2D();
}

FFIVSEdPinConnectDragDrop::FFIVSEdPinConnectDragDrop(TSharedRef<SFIVSEdPinViewer> InPin) : Pin(InPin) {
	Pin->GetNodeViewer()->GetGraphViewer()->BeginDragPin(Pin);
}

void FFIVSEdPinConnectDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	FDragDropOperation::OnDrop(bDropWasHandled, MouseEvent);
	Pin->GetNodeViewer()->GetGraphViewer()->EndDragPin(Pin);
}

void SFIVSEdNodeViewer::Construct(const FArguments& InArgs, SFIVSEdGraphViewer* InGraphViewer, UFIVSNode* InNode) {
	Node = InNode;
	GraphViewer = InGraphViewer;
	OnPinChangeHandle = Node->OnPinChanged.AddLambda([this](EFIVSNodePinChange, UFIVSPin*) {
		ReconstructPins();
	});
}

SFIVSEdNodeViewer::~SFIVSEdNodeViewer() {
	Node->OnPinChanged.Remove(OnPinChangeHandle);
}

FReply SFIVSEdNodeViewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		GetGraphViewer()->SelectionManager.Select(Node, MouseEvent);
		return FReply::Handled().DetectDrag(AsShared(), EKeys::LeftMouseButton);
	}
	return FReply::Handled();
}

FReply SFIVSEdNodeViewer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Handled();
}

FReply SFIVSEdNodeViewer::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFIVSEdNodeDragDrop>(SharedThis(this)));
	}
	return FReply::Unhandled();
}

FReply SFIVSEdNodeViewer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	return FReply::Unhandled();
}

UFIVSNode* SFIVSEdNodeViewer::GetNode() const {
	return Node;
}

FVector2D SFIVSEdNodeViewer::GetPosition() const {
	return Node->Pos;
}

const TArray<TSharedRef<SFIVSEdPinViewer>>& SFIVSEdNodeViewer::GetPinWidgets() const {
	return PinWidgets;
}

TSharedRef<SFIVSEdPinViewer> SFIVSEdNodeViewer::GetPinWidget(UFIVSPin* Pin) const {
	return PinToWidget[Pin];
}

void SFIVSEdRerouteNodeViewer::Construct(const FArguments& InArgs, SFIVSEdGraphViewer* InGraphViewer, UFIVSNode* InNode) {
	SFIVSEdNodeViewer::Construct(SFIVSEdNodeViewer::FArguments(), InGraphViewer, InNode);
	Style = InArgs._Style;
	OutlineBrush = FSlateColorBrush(InArgs._OutlineColor);
	NodeBrush = FSlateColorBrush(InArgs._BackgroundColor);
	
	TSharedPtr<SFIVSEdPinViewer> PinWidget;
	UFIVSPin* Pin = GetNode()->GetNodePins()[0];
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
				SAssignNew(PinWidget, SFIVSEdPinViewer, this, Pin)
				.Style(Style)
			]
		]
	];
	PinWidgets.Add(PinWidget.ToSharedRef());
	PinToWidget.Add(Pin, PinWidget.ToSharedRef());
}

void SFIVSEdFunctionNodeViewer::Construct(const FArguments& InArgs, SFIVSEdGraphViewer* InGraphViewer, UFIVSNode* InNode) {
	SFIVSEdNodeViewer::Construct(SFIVSEdNodeViewer::FArguments(), InGraphViewer, InNode);
	Style = InArgs._Style;
	OutlineBrush = FSlateColorBrush(InArgs._OutlineColor);
	NodeBrush = FSlateColorBrush(InArgs._BackgroundColor);
	HeaderBrush = FSlateColorBrush(InArgs._HeaderColor);
	
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
                            return FText::FromString(Cast<UFIVSScriptNode>(GetNode())->GetNodeName());
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

	ReconstructPins();
}

void SFIVSEdFunctionNodeViewer::ReconstructPins() {
	InputPinBox->ClearChildren(); 
	OutputPinBox->ClearChildren();
	
	for (UFIVSPin* Pin : GetNode()->GetNodePins()) {
		if (Pin->GetPinType() & FIVS_PIN_INPUT) {
			TSharedRef<SFIVSEdPinViewer> PinWidget = SNew(SFIVSEdPinViewer, this, Pin)
			.Style(Style);
			PinWidgets.Add(PinWidget);
			PinToWidget.Add(Pin, PinWidget);
			InputPinBox->AddSlot()
			.AutoHeight()[
				PinWidget
			];
		} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
			TSharedRef<SFIVSEdPinViewer> PinWidget = SNew(SFIVSEdPinViewer, this, Pin)
			.Style(Style);
			PinWidgets.Add(PinWidget);
			PinToWidget.Add(Pin, PinWidget);
			OutputPinBox->AddSlot()
			.AutoHeight()[
				PinWidget
			];
		}
	}
}

void SFIVSEdOperatorNodeViewer::Construct(const FArguments& InArgs, SFIVSEdGraphViewer* InGraphViewer, UFIVSNode* InNode) {
	SFIVSEdNodeViewer::Construct(SFIVSEdNodeViewer::FArguments(), InGraphViewer, InNode);
	Style = InArgs._Style;
	OutlineBrush = FSlateColorBrush(InArgs._OutlineColor);
	NodeBrush = FSlateColorBrush(InArgs._BackgroundColor);
	
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
				SNew(SOverlay)
				+SOverlay::Slot()
				.Padding(10)[
					SNew(SScaleBox)
					.Stretch(EStretch::ScaleToFit)
					.Content()[
						SNew(STextBlock)
						.Text(FText::FromString(InArgs._Symbol))
						.ColorAndOpacity(FLinearColor(1, 1, 1, 0.2))
					]
				]
				+SOverlay::Slot()[
					SNew(SGridPanel)
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
		]
	];

	ReconstructPins();
}

void SFIVSEdOperatorNodeViewer::ReconstructPins() {
	InputPinBox->ClearChildren();
	OutputPinBox->ClearChildren();
	
	for (UFIVSPin* Pin : GetNode()->GetNodePins()) {
		if (Pin->GetPinType() & FIVS_PIN_INPUT) {
			TSharedRef<SFIVSEdPinViewer> PinWidget = SNew(SFIVSEdPinViewer, this, Pin)
			.Style(Style)
			.ShowName(false);
			PinWidgets.Add(PinWidget);
			PinToWidget.Add(Pin, PinWidget);
			InputPinBox->AddSlot()
			.VAlign(VAlign_Center)[
				PinWidget
			];
		} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
			TSharedRef<SFIVSEdPinViewer> PinWidget = SNew(SFIVSEdPinViewer, this, Pin)
			.Style(Style)
			.ShowName(false);
			PinWidgets.Add(PinWidget);
			PinToWidget.Add(Pin, PinWidget);
			OutputPinBox->AddSlot()
			.VAlign(VAlign_Center)[
				PinWidget
			];
		}
	}
}

FFIVSEdNodeDragDrop::FFIVSEdNodeDragDrop(TSharedRef<SFIVSEdNodeViewer> InNodeViewer) : NodeViewer(InNodeViewer) {}

void FFIVSEdNodeDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	SFIVSEdGraphViewer* GraphViewer = NodeViewer->GetGraphViewer();
	FVector2D StartPos = GraphViewer->LocalToGraph(GraphViewer->GetCachedGeometry().AbsoluteToLocal(DragDropEvent.GetLastScreenSpacePosition()));
	FVector2D EndPos = GraphViewer->LocalToGraph(GraphViewer->GetCachedGeometry().AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition()));
	FVector2D Offset = EndPos - StartPos;
	for (UFIVSNode* Node : GraphViewer->SelectionManager.GetSelection()) {
		Node->Pos += Offset;
	}
 }
