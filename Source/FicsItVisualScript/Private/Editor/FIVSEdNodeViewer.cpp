#include "Editor/FIVSEdNodeViewer.h"

#include "Editor/FIVSEdGraphViewer.h"
#include "Script/FIVSScriptNode.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Layout/SScaleBox.h"

void SFIVSEdPinViewer::Construct(const FArguments& InArgs, const TSharedRef<SFIVSEdNodeViewer>& InNodeViewer, UFIVSPin* InPin) {
	Style = InArgs._Style;
	NodeViewer = InNodeViewer;
	Pin = InPin;
	
	PinIconWidget = SNew(SImage)
		.Image_Lambda([this]() {
			if (Pin->GetConnections().Num() > 0) {
				return &Style->DataInputPinStyle.ConnectionIconConnected;
			} else {
				return &Style->DataInputPinStyle.ConnectionIcon;
			}
		})
		.ColorAndOpacity_Raw(this, &SFIVSEdPinViewer::GetPinColor);

	TSharedPtr<SWidget> Content;
	if (Pin->GetDisplayName().IsEmpty()) {
		Content =  PinIconWidget.ToSharedRef();
	} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
		SHorizontalBox::FSlot* NameSlot;
		Content = SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				.AutoWidth()
				.Expose(NameSlot)
			+SHorizontalBox::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.AutoWidth()[
					PinIconWidget.ToSharedRef()
				];
		if (InArgs._ShowName) {
			(*NameSlot)[
				SNew(STextBlock)
				.TextStyle(&Style->PinTextStyle)
				.Margin(Style->PinTextMargin)
				.Text_Lambda([this]() {
					return Pin->GetDisplayName();
				})
			];
		}
	} else if (Pin->GetPinType() & FIVS_PIN_INPUT) {
		TSharedPtr<SWidget> Literal;
		TSharedPtr<SWidget> InlineLiteral;

		switch (Pin->GetPinDataType().GetType()) {
			case FIN_BOOL:
				InlineLiteral = SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([this](ECheckBoxState InState) {
					if (InState == ECheckBoxState::Checked) {
						Pin->SetLiteral(true);
					} else {
						Pin->SetLiteral(false);
					}
				})
				.IsChecked_Lambda([this]() {
					return Pin->GetLiteral().GetBool() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				});
			break;
			case FIN_INT:
				InlineLiteral = SNew(SNumericEntryBox<int>)
				.OnValueChanged_Lambda([this](int Value) {
					Pin->SetLiteral((FINInt)Value);
				})
				.Value_Lambda([this]() {
					return Pin->GetLiteral().GetInt();
				});
			break;
			case FIN_FLOAT:
				InlineLiteral = SNew(SNumericEntryBox<double>)
				.OnValueChanged_Lambda([this](double Value) {
					Pin->SetLiteral((FINFloat)Value);
				})
				.Value_Lambda([this]() {
					return Pin->GetLiteral().GetFloat();
				});
			break;
			case FIN_STR:
				Literal = SNew(SEditableTextBox)
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

		SGridPanel::FSlot* NameSlot;
		SGridPanel::FSlot* InlineLiteralSlot;
		SGridPanel::FSlot* LiteralSlot;
		Content = SNew(SGridPanel)
			+SGridPanel::Slot(0, 0)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)[
				PinIconWidget.ToSharedRef()
			]
			+SGridPanel::Slot(1, 0).Expose(NameSlot)
			+SGridPanel::Slot(2, 0).Expose(InlineLiteralSlot)
			+SGridPanel::Slot(1, 1).Expose(LiteralSlot);

		if (InArgs._ShowName) {
			(*NameSlot)[
				SNew(STextBlock)
				.Margin(Style->PinTextMargin)
				.TextStyle(&Style->PinTextStyle)
				.Text_Lambda([this]() {
					return Pin->GetDisplayName();
				})
			];
		}

		auto visibility = TAttribute<EVisibility>::CreateLambda([this]() {
			if (Pin->GetConnections().Num() > 0) {
				return EVisibility::Collapsed;
			} else {
				return EVisibility::Visible;
			}
		});

		if (InlineLiteral) {
			(*InlineLiteralSlot)[
				InlineLiteral.ToSharedRef()
			];
			InlineLiteral->SetVisibility(visibility);
		}
		if (Literal) {
			(*LiteralSlot)[
				Literal.ToSharedRef()
			];
			Literal->SetVisibility(visibility);
		}
	}

	ChildSlot[
		SNew(SBorder)
		.Padding(Style->PinPadding)
		.BorderImage_Lambda([this]() {
			return IsHovered() ? &Style->PinOutline : nullptr;
		})
		.Content()[
			Content.ToSharedRef()
		]
	];
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

TSharedRef<SBorder> SFIVSEdNodeViewer::Construct(const TSharedRef<SFIVSEdGraphViewer>& InGraphViewer, UFIVSNode* InNode, const FFIVSEdNodeStyle* InStyle) {
	Node = InNode;
	GraphViewer = InGraphViewer;
	Style = InStyle;
	OnPinChangeHandle = Node->OnPinChanged.AddLambda([this](EFIVSNodePinChange, UFIVSPin*) {
		ReconstructPins();
	});

	TSharedPtr<SBorder> contentContainer;
	ChildSlot[
		SAssignNew(contentContainer, SBorder)
		.Padding(Style->OutlinePadding)
		.BorderImage_Lambda([this]() {
			return bSelected ? &Style->Outline : nullptr;
		})
	];
	return contentContainer.ToSharedRef();
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
	return Node->Pos - Style->OutlinePadding.GetTopLeft();
}

const TArray<TSharedRef<SFIVSEdPinViewer>>& SFIVSEdNodeViewer::GetPinWidgets() const {
	return PinWidgets;
}

TSharedRef<SFIVSEdPinViewer> SFIVSEdNodeViewer::GetPinWidget(UFIVSPin* Pin) const {
	return PinToWidget[Pin];
}

void SFIVSEdRerouteNodeViewer::Construct(const FArguments& InArgs, const TSharedRef<SFIVSEdGraphViewer>& InGraphViewer, UFIVSNode* InNode) {
	TSharedRef<SBorder> content = SFIVSEdNodeViewer::Construct(InGraphViewer, InNode, InArgs._Style);

	TSharedPtr<SFIVSEdPinViewer> PinWidget;
	UFIVSPin* Pin = GetNode()->GetNodePins()[0];

	content->SetContent(
		SNew(SBorder)
		.BorderImage(&Style->Background)
		.Padding(Style->Padding)
		.Content()[
			SAssignNew(PinWidget, SFIVSEdPinViewer, SharedThis(this), Pin)
			.Style(Style)
		]
	);
	PinWidgets.Add(PinWidget.ToSharedRef());
	PinToWidget.Add(Pin, PinWidget.ToSharedRef());
}

void SFIVSEdFunctionNodeViewer::Construct(const FArguments& InArgs, const TSharedRef<SFIVSEdGraphViewer>& InGraphViewer, UFIVSNode* InNode) {
	TSharedRef<SBorder> content = SFIVSEdNodeViewer::Construct(InGraphViewer, InNode, InArgs._Style);

	content->SetContent(
        SNew(SVerticalBox)
        +SVerticalBox::Slot().AutoHeight()[
            SNew(SBorder)
            .BorderImage(&Style->Header)
            .Padding(Style->HeaderPadding)
            .Content()[
                SNew(STextBlock)
	            .TextStyle(&Style->HeaderTextStyle)
                .Text_Lambda([this]() {
                	if (auto node = Cast<UFIVSScriptNode>(GetNode())) {
                		return node->DisplayName;
                	} else {
                		return FText::FromString("Unnamed");
                	}
                })
            ]
        ]
        +SVerticalBox::Slot().AutoHeight()[
	        SNew(SBorder)
	        .Padding(Style->Padding)
	        .BorderImage(&Style->Background)
	        .Content()[
		        SNew(SHorizontalBox)
		        +SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)[
		            SAssignNew(InputPinBox, SVerticalBox)
		        ]
		        +SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Fill)[
		            SNew(SSpacer).Size(Style->CenterSpace)
		        ]
		        +SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Right)[
		            SAssignNew(OutputPinBox, SVerticalBox)
		        ]
		    ]
	    ]
    );

	ReconstructPins();
}

bool SFIVSEdFunctionNodeViewer::ShowName() const {
	return true;
}

void SFIVSEdFunctionNodeViewer::ReconstructPins() {
	InputPinBox->ClearChildren(); 
	OutputPinBox->ClearChildren();
	PinWidgets.Empty();
	PinToWidget.Empty();
	
	for (UFIVSPin* Pin : GetNode()->GetNodePins()) {
		if (Pin->GetPinType() & FIVS_PIN_INPUT) {
			TSharedRef<SFIVSEdPinViewer> PinWidget = SNew(SFIVSEdPinViewer, SharedThis(this), Pin)
				.Style(Style)
				.ShowName(ShowName());
			PinWidgets.Add(PinWidget);
			PinToWidget.Add(Pin, PinWidget);
			InputPinBox->AddSlot().AutoHeight().HAlign(HAlign_Left)[
				SNew(SBox)
				.Padding(Style->PinMargin)
				.Content()[
					PinWidget
				]
			];
		} else if (Pin->GetPinType() & FIVS_PIN_OUTPUT) {
			TSharedRef<SFIVSEdPinViewer> PinWidget = SNew(SFIVSEdPinViewer, SharedThis(this), Pin)
				.Style(Style)
				.ShowName(ShowName());
			PinWidgets.Add(PinWidget);
			PinToWidget.Add(Pin, PinWidget);
			OutputPinBox->AddSlot().AutoHeight().HAlign(HAlign_Right)[
				SNew(SBox)
				.Padding(Style->PinMargin)
				.Content()[
					PinWidget
				]
			];
		}
	}
}

void SFIVSEdOperatorNodeViewer::Construct(const FArguments& InArgs, const TSharedRef<SFIVSEdGraphViewer>& InGraphViewer, UFIVSNode* InNode) {
	TSharedRef<SBorder> content = SFIVSEdNodeViewer::Construct(InGraphViewer, InNode, InArgs._Style);

	content->SetContent(
		SNew(SBorder)
		.BorderImage(&Style->Background)
		.Padding(Style->Padding)
		.Content()[
			SNew(SOverlay)
			+SOverlay::Slot()
			.Padding(Style->OperatorPadding)[
				SNew(SScaleBox)
				.Stretch(EStretch::ScaleToFit)
				.Content()[
					SNew(STextBlock)
					.TextStyle(&Style->OperatorTextStyle)
					.Text(FText::FromString(InArgs._Symbol))
				]
			]
			+SOverlay::Slot()[
				SNew(SGridPanel)
				+SGridPanel::Slot(0, 1).HAlign(HAlign_Left).VAlign(VAlign_Center)[
					SAssignNew(InputPinBox, SVerticalBox)
				]
				+SGridPanel::Slot(1, 1).HAlign(HAlign_Center)[
					SNew(SSpacer).Size(Style->CenterSpace)
				]
				+SGridPanel::Slot(2, 1).HAlign(HAlign_Right).VAlign(VAlign_Center)[
					SAssignNew(OutputPinBox, SVerticalBox)
				]
			]
		]
	);

	ReconstructPins();
}

bool SFIVSEdOperatorNodeViewer::ShowName() const {
	return false;
}

FFIVSEdNodeDragDrop::FFIVSEdNodeDragDrop(TSharedRef<SFIVSEdNodeViewer> InNodeViewer) : NodeViewer(InNodeViewer) {
	TSharedPtr<SFIVSEdGraphViewer> GraphViewer = NodeViewer->GetGraphViewer();
	Nodes = GraphViewer->SelectionManager.GetSelection();
	for (UFIVSNode* node : Nodes) {
		NodeStartPos.Add(node->Pos);
	}
}

void FFIVSEdNodeDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	TSharedPtr<SFIVSEdGraphViewer> GraphViewer = NodeViewer->GetGraphViewer();
	FVector2D graphPos = GraphViewer->LocalToGraph(GraphViewer->GetCachedGeometry().AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition()));
	if (PreviousGraphPos) {
		Delta += graphPos - *PreviousGraphPos;

		for (int32 i = 0; i < Nodes.Num(); ++i) {
			UFIVSNode* Node = Nodes[i];
			FVector2D startPos = NodeStartPos[i];
			Node->Pos = startPos + Delta;
			if (!DragDropEvent.IsShiftDown()) {
				Node->Pos = (Node->Pos / 10).RoundToVector() * 10;
			}
		}
	}
	PreviousGraphPos = graphPos;
 }
