#include "Editor/FIVSEdGraphViewer.h"

#include "FicsItVisualScriptModule.h"
#include "Script/FIVSGraph.h"
#include "Reflection/FINReflection.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

void FFIVSEdConnectionDrawer::Reset() {
	ConnectionUnderMouse = TPair<TSharedPtr<SFIVSEdPinViewer>, TSharedPtr<SFIVSEdPinViewer>>(nullptr, nullptr);
	LastConnectionDistance = FLT_MAX;
}

void FFIVSEdConnectionDrawer::DrawConnection(TSharedRef<SFIVSEdPinViewer> Pin1, TSharedRef<SFIVSEdPinViewer> Pin2, TSharedRef<const SFIVSEdGraphViewer> Graph, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) {
	bool is1Wild = !!Cast<UFIVSWildcardPin>(Pin1->GetPin());
	bool is2Wild = !!Cast<UFIVSWildcardPin>(Pin2->GetPin());

	bool bShouldSwitch = false;
	if (is1Wild) {
		if (is2Wild) {
			bool bHasInput = false;
			for (UFIVSPin* Con : Pin1->GetPin()->GetConnections()) if (Con->GetPinType() & FIVS_PIN_INPUT) {
				bHasInput = true;
				break;
			}
			if (Pin1->GetPin()->ParentNode->Pos.X > Pin2->GetPin()->ParentNode->Pos.X && bHasInput) {
				bShouldSwitch = true;
			}
		} else if (!(Pin2->GetPin()->GetPinType() & FIVS_PIN_INPUT)) {
			bShouldSwitch = true;
		}
	} else if (is2Wild) {
		if (!(Pin1->GetPin()->GetPinType() & FIVS_PIN_OUTPUT)) {
			bShouldSwitch = true;
		}
	} else if (!(Pin1->GetPin()->GetPinType() & FIVS_PIN_OUTPUT)) {
		bShouldSwitch = true;
	}
	if (bShouldSwitch) {
		TSharedRef<SFIVSEdPinViewer> Pin = Pin1;
		Pin1 = Pin2;
		Pin2 = Pin;
	}
	FVector2D StartLoc = Graph->GetCachedGeometry().AbsoluteToLocal(Pin1->GetConnectionPoint());
	FVector2D EndLoc = Graph->GetCachedGeometry().AbsoluteToLocal(Pin2->GetConnectionPoint());
	DrawConnection(StartLoc, EndLoc, Pin1->GetPinColor().GetSpecifiedColor(), Graph, AllottedGeometry, OutDrawElements, LayerId);

	// Find the closest approach to the spline
	FVector2D ClosestPoint;
	float ClosestDistanceSquared = FLT_MAX;

	const int32 NumStepsToTest = 16;
	const float StepInterval = 1.0f / (float)NumStepsToTest;
	FVector2D Point1 = FMath::CubicInterp(StartLoc, FVector2D(300, 0), EndLoc, FVector2D(300, 0), 0.0f);
	for (float t = 0.0f; t < 1.0f; t += StepInterval) {
		const FVector2D Point2 = FMath::CubicInterp(StartLoc, FVector2D(300, 0), EndLoc, FVector2D(300, 0), t + StepInterval);

		const FVector2D ClosestPointToSegment = FMath::ClosestPointOnSegment2D(Graph->GetCachedGeometry().AbsoluteToLocal(LastMousePosition), Point1, Point2);
		const float DistanceSquared = (Graph->GetCachedGeometry().AbsoluteToLocal(LastMousePosition) - ClosestPointToSegment).SizeSquared();

		if (DistanceSquared < ClosestDistanceSquared) {
			ClosestDistanceSquared = DistanceSquared;
			ClosestPoint = ClosestPointToSegment;
		}

		Point1 = Point2;
	}
	if (ClosestDistanceSquared < LastConnectionDistance && ClosestDistanceSquared < 100) {
		ConnectionUnderMouse = TPair<TSharedPtr<SFIVSEdPinViewer>, TSharedPtr<SFIVSEdPinViewer>>(Pin1, Pin2);
		LastConnectionDistance = ClosestDistanceSquared;
	}
}


void FFIVSEdConnectionDrawer::DrawConnection(const FVector2D& Start, const FVector2D& End, const FLinearColor& ConnectionColor, TSharedRef<const SFIVSEdGraphViewer> Graph, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) {
	//FSlateDrawElement::MakeSpline(OutDrawElements, LayerId+100, AllottedGeometry.ToPaintGeometry(), Start, FVector2D(300 * Graph->Zoom,0), End, FVector2D(300 * Graph->Zoom,0), 2 * Graph->Zoom, ESlateDrawEffect::None, ConnectionColor);
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId+100, AllottedGeometry.ToPaintGeometry(), {Start, End}, ESlateDrawEffect::None, ConnectionColor, true, 2 * Graph->GetZoom());
}

void SFIVSEdGraphViewer::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	SetGraph(InArgs._Graph);
	SelectionChanged = InArgs._OnSelectionChanged;
	SelectionManager.OnSelectionChanged.BindRaw(this, &SFIVSEdGraphViewer::OnSelectionChanged);
}

void SFIVSEdGraphViewer::OnSelectionChanged(UFIVSNode* InNode, bool bIsSelected) {
	NodeToChild[InNode]->bSelected = bIsSelected;
	SelectionChanged.ExecuteIfBound(InNode, bIsSelected);
}

SFIVSEdGraphViewer::SFIVSEdGraphViewer() : Children(this) {
	ConnectionDrawer = MakeShared<FFIVSEdConnectionDrawer>();
}

SFIVSEdGraphViewer::~SFIVSEdGraphViewer() {
	if (Graph) Graph->OnNodeChanged.Remove(OnNodeChangedHandle);
}

FVector2D SFIVSEdGraphViewer::ComputeDesiredSize(float) const {
	return FVector2D(160.0f, 120.0f);
}

#pragma optimize("", off)
int32 SFIVSEdGraphViewer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry));
	
	ConnectionDrawer->Reset();
	int ret = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+2, InWidgetStyle, bParentEnabled);

	// Draw Grid
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), &BackgroundBrush, ESlateDrawEffect::None, BackgroundBrush.TintColor.GetSpecifiedColor());
	
	FVector2D Distance = FVector2D(10,10);
	FVector2D Start = Offset / Distance;
	FVector2D RenderOffset = FVector2D(FMath::Fractional(Start.X) * Distance.X, FMath::Fractional(Start.Y) * Distance.Y);
	int GridOffsetX = FMath::FloorToInt(Start.X) * FMath::RoundToInt(Distance.X);
	int GridOffsetY = FMath::FloorToInt(Start.Y) * FMath::RoundToInt(Distance.Y);
	Distance *= GetZoom();
	Start *= GetZoom();
	RenderOffset *= GetZoom();
	for (float x = 0; x <= AllottedGeometry.GetLocalSize().X; x += Distance.X) {
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(x + RenderOffset.X, 0), FVector2D(x + RenderOffset.X, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, GridColor, true, ((FMath::RoundToInt(x/GetZoom()) - GridOffsetX) % 100 == 0) ? 1.0 : 0.05);
	}
	for (float y = 0; y <= AllottedGeometry.GetLocalSize().Y; y += Distance.Y) {
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(0, y + RenderOffset.Y), FVector2D(AllottedGeometry.GetLocalSize().X, y + RenderOffset.Y)}, ESlateDrawEffect::None, GridColor, true, ((FMath::RoundToInt(y/GetZoom()) - GridOffsetY) % 100 == 0) ? 1.0 : 0.05);
	}

	// Draw Pin Connections
	TMap<TSharedRef<SFIVSEdPinViewer>, FVector2D> ConnectionLocations; 
	TMap<UFIVSPin*, TSharedRef<SFIVSEdPinViewer>> PinMap;
	
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFIVSEdNodeViewer> Node = Children[i];
		for (int j = 0; j < Node->GetPinWidgets().Num(); ++j) {
			TSharedRef<SFIVSEdPinViewer> Pin = Node->GetPinWidgets()[j];
			ConnectionLocations.Add(Pin, Pin->GetConnectionPoint());
			PinMap.Add(Pin->GetPin(), Pin);
		}
	}

	TSet<TPair<UFIVSPin*, UFIVSPin*>> DrawnPins;
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFIVSEdNodeViewer> Node = Children[i];
		for (const TSharedRef<SFIVSEdPinViewer>& Pin : Node->GetPinWidgets()) {
			if (Pin->GetPin()->IsValidLowLevel()) for (UFIVSPin* ConnectionPin : Pin->GetPin()->GetConnections()) {
				if (!DrawnPins.Contains(TPair<UFIVSPin*, UFIVSPin*>(Pin->GetPin(), ConnectionPin)) && !DrawnPins.Contains(TPair<UFIVSPin*, UFIVSPin*>(ConnectionPin, Pin->GetPin()))) {
					DrawnPins.Add(TPair<UFIVSPin*, UFIVSPin*>(Pin->GetPin(), ConnectionPin));
					ConnectionDrawer->DrawConnection(Pin, NodeToChild[ConnectionPin->ParentNode]->GetPinWidget(ConnectionPin), SharedThis(this), AllottedGeometry, OutDrawElements, LayerId+1);
				}
			} else {
				UE_LOG(LogFicsItVisualScript, Warning, TEXT("WTF"));
			}
		}
	}

	// draw new creating connection
	for (TSharedRef<SFIVSEdPinViewer> DraggingPin : DraggingPins) {
		FVector2D StartLoc = GetCachedGeometry().AbsoluteToLocal(DraggingPin->GetConnectionPoint());
		FVector2D EndLoc = GetCachedGeometry().AbsoluteToLocal(DraggingPinsEndpoint);
		if (DraggingPin->GetPin()->GetPinType() & FIVS_PIN_INPUT) {
			Swap(EndLoc, StartLoc);
		}
		ConnectionDrawer->DrawConnection(StartLoc, EndLoc, DraggingPin->GetPinColor().GetSpecifiedColor(), SharedThis(this), AllottedGeometry, OutDrawElements, LayerId+100);
	}
	
	// Draw selection box
	if (SelectionBoxHandler.IsActive()) {
		FVector2D SelectStart;
		FVector2D SelectEnd;
		SelectionBoxHandler.GetRect(SelectStart, SelectEnd);
		SelectStart = GetCachedGeometry().AbsoluteToLocal(SelectStart);
		SelectEnd = GetCachedGeometry().AbsoluteToLocal(SelectEnd);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId+200, AllottedGeometry.ToPaintGeometry(SelectStart, SelectEnd - SelectStart), &SelectionBrush, ESlateDrawEffect::None, FLinearColor(1,1,1,0.1));
	}

	OutDrawElements.PopClip();
	
	return ret;
}
#pragma optimize("", off)

FReply SFIVSEdGraphViewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (ConnectionDrawer->ConnectionUnderMouse.Key.IsValid() && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		TPair<TSharedPtr<SFIVSEdPinViewer>, TSharedPtr<SFIVSEdPinViewer>> Connection = ConnectionDrawer->ConnectionUnderMouse;
		TSharedPtr<IMenu> MenuHandle;
		FMenuBuilder MenuBuilder(true, NULL);
		MenuBuilder.AddMenuEntry(
            FText::FromString("Remove Connection"),
            FText(),
            FSlateIcon(),
            FUIAction(FExecuteAction::CreateLambda([Connection]() {
                Connection.Key->GetPin()->RemoveConnection(Connection.Value->GetPin());
            })));
		
		FSlateApplication::Get().PushMenu(SharedThis(this), *MouseEvent.GetEventPath(), MenuBuilder.MakeWidget(), MouseEvent.GetScreenSpacePosition(), FPopupTransitionEffect::ContextMenu);
	} else if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	} else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
		bIsGraphDrag = true;
		GraphDragDelta = 0.0f;
		return FReply::Handled().CaptureMouse(AsShared());
	}
	return FReply::Unhandled();
}

FReply SFIVSEdGraphViewer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (bIsGraphDrag) {
		bIsGraphDrag = false;
		if (GraphDragDelta < 10 && Graph) {
			CreateActionSelectionMenu(*MouseEvent.GetEventPath(), MouseEvent.GetScreenSpacePosition(), [this](auto){}, FFINScriptNodeCreationContext(Graph, LocalToGraph(GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())), nullptr));
		}
		return FReply::Handled().ReleaseMouseCapture();
	} else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
		CreateActionSelectionMenu(*MouseEvent.GetEventPath(), MouseEvent.GetScreenSpacePosition(), [this](auto){}, FFINScriptNodeCreationContext(Graph, LocalToGraph(GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())), nullptr));
		return FReply::Handled();
	} else if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		SelectionManager.DeselectAll();
		return FReply::Handled();
	}
	return SPanel::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SFIVSEdGraphViewer::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && ConnectionDrawer.IsValid() && ConnectionDrawer->ConnectionUnderMouse.Key) {
		TPair<TSharedPtr<SFIVSEdPinViewer>, TSharedPtr<SFIVSEdPinViewer>> Connection = ConnectionDrawer->ConnectionUnderMouse;
		UFIVSPin* Pin1 = Connection.Key->GetPin();
		UFIVSPin* Pin2 = Connection.Value->GetPin();
		UFIVSRerouteNode* Node = NewObject<UFIVSRerouteNode>();
		Node->Pos = LocalToGraph(GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition())) - FVector2D(10, 10);
		Pin1->RemoveConnection(Connection.Value->GetPin());
		Pin1->AddConnection(Node->GetNodePins()[0]);
		Pin2->AddConnection(Node->GetNodePins()[0]);
		Graph->AddNode(Node);
		return FReply::Handled();
	}
	return SPanel::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SFIVSEdGraphViewer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (ConnectionDrawer) ConnectionDrawer->LastMousePosition = MouseEvent.GetScreenSpacePosition();
	if (bIsGraphDrag) {
		FVector2D Delta = (MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition())) / GetZoom();
		Offset += Delta;
		GraphDragDelta += MouseEvent.GetCursorDelta().Size();
		return FReply::Handled();
	}
	return SPanel::OnMouseMove(MyGeometry, MouseEvent);
}

FReply SFIVSEdGraphViewer::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D StartPos = LocalToGraph(GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	Zoom += MouseEvent.GetWheelDelta()/10;
	if (Zoom < 0.1) Zoom = 0.1;
	if (Zoom > 5) Zoom = 5;
	FVector2D EndPos = LocalToGraph(GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	FVector2D Off = StartPos - EndPos;
	Offset -= Off;
	return FReply::Handled();
}

FReply SFIVSEdGraphViewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Delete) {
		if (SelectionManager.GetSelection().Num() > 0) {
			for (UFIVSNode* Node : SelectionManager.GetSelection()) {
				Graph->RemoveNode(Node);
			}
		} else if (ConnectionDrawer && ConnectionDrawer->ConnectionUnderMouse.Key) {
			ConnectionDrawer->ConnectionUnderMouse.Key->GetPin()->RemoveConnection(ConnectionDrawer->ConnectionUnderMouse.Value->GetPin());
		}
	} else if (InKeyEvent.GetKey() == EKeys::C && InKeyEvent.IsControlDown()) {
		FWindowsPlatformApplicationMisc::ClipboardCopy(*UFIVSSerailizationUtils::FIVS_SerializePartial(SelectionManager.GetSelection(), true));
	} else if (InKeyEvent.GetKey() == EKeys::V && InKeyEvent.IsControlDown()) {
		FString Paste;
		FWindowsPlatformApplicationMisc::ClipboardPaste(Paste);
		UFIVSSerailizationUtils::FIVS_DeserializeGraph(Graph, Paste, LocalToGraph(MyGeometry.AbsoluteToLocal(FSlateApplication::Get().GetCursorPos())));
	}
	return SPanel::OnKeyDown(MyGeometry, InKeyEvent);
}
FReply SFIVSEdGraphViewer::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	return SPanel::OnKeyUp(MyGeometry, InKeyEvent);
}

FReply SFIVSEdGraphViewer::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		return FReply::Handled().BeginDragDrop(MakeShared<FFIVSEdSelectionBoxDragDrop>(SelectionBoxHandler, MouseEvent.GetScreenSpacePosition()));
	}
	return FReply::Unhandled();
}

FReply SFIVSEdGraphViewer::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (Operation->IsOfType<FFIVSEdPinConnectDragDrop>()) {
		DraggingPinsEndpoint = DragDropEvent.GetScreenSpacePosition();
		return FReply::Handled();
	}
	if (Operation->IsOfType<FFIVSEdSelectionBoxDragDrop>()) {
		TSharedPtr<FFIVSEdSelectionBoxDragDrop> SelectionBox = DragDropEvent.GetOperationAs<FFIVSEdSelectionBoxDragDrop>();
		FFIVSEdSelectionBoxHandler& BoxHandler = SelectionBox->GetSelectionBoxHandler();
		BoxHandler.UpdateDrag(DragDropEvent.GetScreenSpacePosition());
		
		FVector2D LocalStart;
		FVector2D LocalEnd;
		BoxHandler.GetRect(LocalStart, LocalEnd);
		LocalStart = GetCachedGeometry().AbsoluteToLocal(LocalStart);
		LocalEnd = GetCachedGeometry().AbsoluteToLocal(LocalEnd);

		FSlateRect SelectionRect = FSlateRect(FVector2D(FMath::Min(LocalStart.X, LocalEnd.X), FMath::Min(LocalStart.Y, LocalEnd.Y)), FVector2D(FMath::Max(LocalStart.X, LocalEnd.X), FMath::Max(LocalStart.Y, LocalEnd.Y)));
		
		if (Graph) {
			TArray<UFIVSNode*> SelectedNodes;
			for (UFIVSNode* Node : Graph->GetNodes()) {
				TSharedRef<SFIVSEdNodeViewer> NodeW = NodeToChild[Node];

				FSlateRect NodeRect = FSlateRect(GetCachedGeometry().AbsoluteToLocal(NodeW->GetCachedGeometry().GetAbsolutePosition()), GetCachedGeometry().AbsoluteToLocal(NodeW->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(1,1))));

				if (Node && FSlateRect::DoRectanglesIntersect(NodeRect, SelectionRect)) {
					SelectedNodes.Add(Node);
				}
			}
			SelectionManager.Select(SelectedNodes, DragDropEvent);
		}
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SFIVSEdGraphViewer::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
	TSharedPtr<FFIVSEdPinConnectDragDrop> PinConnect = DragDropEvent.GetOperationAs<FFIVSEdPinConnectDragDrop>();
	if (PinConnect.IsValid()) {
		int UserIndex = DragDropEvent.GetUserIndex();
		CreateActionSelectionMenu(*DragDropEvent.GetEventPath(), DragDropEvent.GetScreenSpacePosition(), [this, UserIndex](auto) {
			FSlateApplication::Get().GetUser(UserIndex)->CancelDragDrop();
		}, FFINScriptNodeCreationContext(Graph, LocalToGraph(GetCachedGeometry().AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition())), PinConnect->GetPinViewer()->GetPin()));
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

bool SFIVSEdGraphViewer::IsInteractable() const {
	return true;
}

bool SFIVSEdGraphViewer::SupportsKeyboardFocus() const {
	return true;
}

FChildren* SFIVSEdGraphViewer::GetChildren() {
	return &Children;
}

void SFIVSEdGraphViewer::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	for (int32 NodeIndex = 0; NodeIndex < Children.Num(); ++NodeIndex) {
		const TSharedRef<SFIVSEdNodeViewer>& Node = Children[NodeIndex];
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Node, Node->GetPosition() + Offset, Node->GetDesiredSize(), GetZoom()));
	}
}

void SFIVSEdGraphViewer::OnNodeChanged(int change, UFIVSNode* Node) {
	if (change == 0) {
		CreateNodeAsChild(Node);
	} else {
		SelectionManager.SetSelected(Node, false);
		TSharedRef<SFIVSEdNodeViewer>* Viewer = NodeToChild.Find(Node);
		if (Viewer) Children.Remove(*Viewer);
		NodeToChild.Remove(Node);
	}
}
#pragma optimize("", off)
TSharedPtr<IMenu> SFIVSEdGraphViewer::CreateActionSelectionMenu(const FWidgetPath& Path, const FVector2D& Location, TFunction<void(const TSharedPtr<FFIVSEdActionSelectionAction>&)> OnExecute, const FFINScriptNodeCreationContext& Context) {
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Entries;
	TMap<FString, TSharedPtr<FFIVSEdActionSelectionCategory>> Categories;
	
	for(TObjectIterator<UClass> It; It; ++It) {
		if(It->IsChildOf(UFIVSNode::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract)) {
			TArray<FFIVSNodeAction> Actions;
			GetDefault<UFIVSNode>(*It)->GetNodeActions(Actions);
			for (const FFIVSNodeAction& Action : Actions) {
				FString CategoryStr = Action.Category.ToString();
				TSharedPtr<FFIVSEdActionSelectionCategory> Category;
				int Sepperator = INDEX_NONE;
				if (CategoryStr.Len() > 0) do {
					int NewSepperator = CategoryStr.Find(TEXT("|"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Sepperator+1);
					FString SubCategory;
					FString FullCategory;
					if (NewSepperator != INDEX_NONE) {
						SubCategory = CategoryStr.Mid(Sepperator, NewSepperator - Sepperator);
						FullCategory = CategoryStr.Left(NewSepperator);
					} else {
						SubCategory = CategoryStr.Mid(Sepperator+1);
						FullCategory = CategoryStr;
					}
					Sepperator = NewSepperator;
					if (SubCategory.Len() < 1) continue;
					TSharedPtr<FFIVSEdActionSelectionCategory>* CategoryPtr = Categories.Find(FullCategory);
					if (!CategoryPtr) {
						TSharedPtr<FFIVSEdActionSelectionCategory> NewCategory = MakeShared<FFIVSEdActionSelectionCategory>(FText::FromString(SubCategory), TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>());
						if (Category.IsValid()) Category->Children.Add(NewCategory);
						else Entries.Add(NewCategory);
						Categories.Add(FullCategory, NewCategory);
						Category = NewCategory;
					} else {
						Category = *CategoryPtr;
					}
				} while (Sepperator != INDEX_NONE);

				TSharedPtr<FFIVSEdActionSelectionNodeAction> SelectionAction = MakeShared<FFIVSEdActionSelectionNodeAction>(Action, Context);
				
				if (Category.IsValid()) {
					Category->Children.Add(SelectionAction);
				} else {
					Entries.Add(SelectionAction);
				}
			}
		}
	}
	
    TSharedRef<SFIVSEdActionSelection> Select = SNew(SFIVSEdActionSelection, Context.Pin ? FFIVSFullPinType(Context.Pin->GetPinType(), Context.Pin->GetPinDataType()) : FFIVSFullPinType(FIVS_PIN_DATA_INPUT & FIVS_PIN_EXEC_OUTPUT, FFIVSPinDataType(FIN_NIL)))
		.OnActionExecuted_Lambda([this, OnExecute](const TSharedPtr<FFIVSEdActionSelectionAction>& Action) {
		OnExecute(Action);
    	ActiveActionSelection = nullptr;
    });
    Select->SetSource(Entries);
    TSharedPtr<IMenu> Menu = FSlateApplication::Get().PushMenu(SharedThis(this), FWidgetPath(), Select, Location, FPopupTransitionEffect::None);
	Select->SetMenu(Menu);
    Select->SetFocus();
	ActiveActionSelection = Select;
	Menu->GetOnMenuDismissed().AddLambda([this, OnExecute](TSharedRef<IMenu>) {
		ActiveActionSelection = nullptr;
		OnExecute(nullptr);
	});
	return Menu;
}
#pragma optimize("", on)

void SFIVSEdGraphViewer::SetGraph(UFIVSGraph* NewGraph) {
	if (Graph) {
		Graph->OnNodeChanged.Remove(OnNodeChangedHandle);
		Children.Empty();
		NodeToChild.Empty();
	}
	
	Graph = NewGraph;

	if (Graph) {
		OnNodeChangedHandle = Graph->OnNodeChanged.AddRaw(this, &SFIVSEdGraphViewer::OnNodeChanged);
		
		// Generate Nodes Children
		for (UFIVSNode* Node : Graph->GetNodes()) {
			CreateNodeAsChild(Node);
		}
	}
}

void SFIVSEdGraphViewer::CreateNodeAsChild(UFIVSNode* Node) {
	TSharedRef<SFIVSEdNodeViewer> Child = Node->CreateNodeViewer(this, Style);
	Children.Add(Child);
	NodeToChild.Add(Node, Child);
}

FVector2D SFIVSEdGraphViewer::LocalToGraph(const FVector2D Local) {
	return (Local / GetZoom()) - Offset;
}

void FFIVSEdSelectionBoxDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	SelectionBoxHandler.EndDrag();
}
