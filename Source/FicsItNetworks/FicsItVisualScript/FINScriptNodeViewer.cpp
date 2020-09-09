#include "FINScriptNodeViewer.h"

void SFINScriptPinViewer::Construct(const FArguments& InArgs) {
	SetPin(InArgs._Pin.Get());
}

SFINScriptPinViewer::SFINScriptPinViewer() : Children(this) {}

FVector2D SFINScriptPinViewer::ComputeDesiredSize(float) const {
	return Children[0]->GetDesiredSize();
}

FChildren* SFINScriptPinViewer::GetChildren() {
	return &Children;
}

void SFINScriptPinViewer::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Children[0], FVector2D(), Children[0]->GetDesiredSize(), 1));
}

FReply SFINScriptPinViewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		TSharedPtr<IMenu> MenuHandle;
		FMenuBuilder MenuBuilder(true, NULL);
		MenuBuilder.AddMenuEntry(
			FText::FromString("Remove Connections"),
            FText(),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([this]() {
				TArray<FFINScriptPin*> Pins = Pin->GetConnections();
	            for (FFINScriptPin* Pin : Pins) Pin->RemoveConnection(GetPin().Get());
			})));
		
		FSlateApplication::Get().PushMenu(SharedThis(this), *MouseEvent.GetEventPath(), MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FSlateColor SFINScriptPinViewer::GetPinColor() const {
	if (Pin->PinType & FIVS_PIN_DATA) {
		switch (Pin->DataType) {
		case FIN_BOOL:		return FLinearColor(FColor::FromHex("FF0000"));
		case FIN_CLASS:		return FLinearColor(FColor::FromHex("AA00AA"));
		case FIN_FLOAT:		return FLinearColor(FColor::FromHex("00FF00"));
		case FIN_INT:		return FLinearColor(FColor::FromHex("77FF77"));
		case FIN_OBJ:		return FLinearColor(FColor::FromHex("0000FF"));
		case FIN_STR:		return FLinearColor(FColor::FromHex("FFAA00"));
		case FIN_STRUCT:	return FLinearColor(FColor::FromHex("000077"));
		case FIN_TRACE:		return FLinearColor(FColor::FromHex("FF4400"));
		};
	}
	return FLinearColor(FColor::White);
}

void SFINScriptPinViewer::SetPin(TSharedPtr<FFINScriptPin> newPin) {
	Children.Empty();
	Pin = newPin;
	if (Pin) {
		if (Pin->PinType & FIVS_PIN_INPUT) {
			Children.Add(SNew(SBorder)
			.BorderBackgroundColor_Lambda([this]() {
                return IsHovered() ? FLinearColor(FColor::White) : FColor::Transparent;
            })
			.Content()[
	            (PinIconContainer = SNew(SHorizontalBox)
	            +SHorizontalBox::Slot().Padding(1).VAlign(EVerticalAlignment::VAlign_Center)[
	                (PinIconWidget = SNew(SImage)
	                .ColorAndOpacity_Raw(this, &SFINScriptPinViewer::GetPinColor)).ToSharedRef()
	            ]
	            +SHorizontalBox::Slot().Padding(5).VAlign(EVerticalAlignment::VAlign_Center)[
	                SNew(STextBlock)
	                .Clipping(EWidgetClipping::Inherit)
	                .Justification(ETextJustify::Left)
	                .Text_Lambda([this]() {
	                    return FText::FromString(Pin->Name);
	                })
	            ]).ToSharedRef()
			]);
		} else if (Pin->PinType & FIVS_PIN_OUTPUT) {
			Children.Add(
                SNew(SBorder)
                .BorderBackgroundColor_Lambda([this]() {
	                return IsHovered() ? FLinearColor(FColor::White) : FColor::Transparent;
                })
                .Content()[(PinIconContainer = SNew(SHorizontalBox)
	                +SHorizontalBox::Slot().Padding(5).VAlign(EVerticalAlignment::VAlign_Center)[
	                    SNew(STextBlock)
						.Clipping(EWidgetClipping::Inherit)
	                    .Justification(ETextJustify::Left)
	                    .Text_Lambda([this]() {
	                        return FText::FromString(Pin->Name);
	                    })
	                ]
	                +SHorizontalBox::Slot().Padding(1).VAlign(EVerticalAlignment::VAlign_Center)[
	                    (PinIconWidget = SNew(SImage)
	                    .ColorAndOpacity_Raw(this, &SFINScriptPinViewer::GetPinColor)).ToSharedRef()
	                ]).ToSharedRef()
				]);
		}
	}
}

TSharedPtr<FFINScriptPin> SFINScriptPinViewer::GetPin() const {
	return Pin;
}

FVector2D SFINScriptPinViewer::GetConnectionPoint() const {
	if (PinIconWidget.IsValid()) return PinIconWidget->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(0.5, 0.5));
	return FVector2D();
}

void SFINScriptNodeViewer::Construct(const FArguments& InArgs) {
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
							if (Node) return FText::FromString(Node->Name);
							return FText::FromString("Undefined");
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
	
	SetNode(InArgs._Node.Get());
}

SFINScriptNodeViewer::SFINScriptNodeViewer() : Children(this) {}

FVector2D SFINScriptNodeViewer::ComputeDesiredSize(float) const {
	FVector2D Size = Children[0]->GetDesiredSize();
	if (Size.X < 100) Size.X = 100;
	if (Size.Y < 100) Size.Y = 100;
	return Size;
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
	InputPinBox->ClearChildren();
	OutputPinBox->ClearChildren();
	PinWidgets.Empty();
	PinToWidget.Empty();

	Node = newNode;

	for (const TSharedPtr<FFINScriptPin>& Pin : newNode->GetPins()) {
		if (Pin->PinType & FIVS_PIN_INPUT) {
			TSharedRef<SFINScriptPinViewer> PinWidget = SNew(SFINScriptPinViewer)
                .Pin(Pin);
			PinWidgets.Add(PinWidget);
			PinToWidget.Add(Pin, PinWidget);
			InputPinBox->AddSlot()[
                PinWidget
            ];
		} else if (Pin->PinType & FIVS_PIN_OUTPUT) {
			TSharedRef<SFINScriptPinViewer> PinWidget = SNew(SFINScriptPinViewer)
                .Pin(Pin);
			PinWidgets.Add(PinWidget);
			PinToWidget.Add(Pin, PinWidget);
			OutputPinBox->AddSlot()[
				PinWidget
			];
		}
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
