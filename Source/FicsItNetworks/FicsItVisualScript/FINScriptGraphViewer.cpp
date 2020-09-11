#include "FINScriptGraphViewer.h"

void FFINScriptConnectionDrawer::Reset() {
	ConnectionUnderMouse = TPair<TSharedPtr<SFINScriptPinViewer>, TSharedPtr<SFINScriptPinViewer>>(nullptr, nullptr);
	LastConnectionDistance = FLT_MAX;
}

void FFINScriptConnectionDrawer::DrawConnection(TSharedRef<SFINScriptPinViewer> Pin1, TSharedRef<SFINScriptPinViewer> Pin2, TSharedRef<const SFINScriptGraphViewer> Graph, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) {
	bool is1Wild = dynamic_cast<FFINScriptWildcardPin*>(Pin1->GetPin().Get());
	bool is2Wild = dynamic_cast<FFINScriptWildcardPin*>(Pin2->GetPin().Get());

	bool bShouldSwitch = false;
	if (is1Wild) {
		if (is2Wild) {
			bool bHasInput = false;
			for (const TSharedPtr<FFINScriptPin>& Con : Pin1->GetPin()->GetConnections()) if (Con->GetPinType() & FIVS_PIN_INPUT) {
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
		TSharedRef<SFINScriptPinViewer> Pin = Pin1;
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
		ConnectionUnderMouse = TPair<TSharedPtr<SFINScriptPinViewer>, TSharedPtr<SFINScriptPinViewer>>(Pin1, Pin2);
		LastConnectionDistance = ClosestDistanceSquared;
	}
}

void FFINScriptConnectionDrawer::DrawConnection(const FVector2D& Start, const FVector2D& End, const FLinearColor& ConnectionColor, TSharedRef<const SFINScriptGraphViewer> Graph, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) {
	FSlateDrawElement::MakeSpline(OutDrawElements, LayerId+100, AllottedGeometry.ToPaintGeometry(), Start, FVector2D(300,0), End, FVector2D(300,0), 2, ESlateDrawEffect::None, ConnectionColor);
}

void SFINScriptGraphViewer::Construct(const FArguments& InArgs) {
	SetGraph(InArgs._Graph.Get());
}

SFINScriptGraphViewer::SFINScriptGraphViewer() : Children(this) {
	ConnectionDrawer = MakeShared<FFINScriptConnectionDrawer>();
}

SFINScriptGraphViewer::~SFINScriptGraphViewer() {
	if (Graph) Graph->OnNodeChanged.Remove(OnNodeChangedHandle);
}

FVector2D SFINScriptGraphViewer::ComputeDesiredSize(float) const {
	return FVector2D(1000,1000);
}

int32 SFINScriptGraphViewer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	ConnectionDrawer->Reset();
	int ret = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+2, InWidgetStyle, bParentEnabled);

	// Draw Grid
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), &BackgroundBrush, ESlateDrawEffect::None, BackgroundBrush.TintColor.GetSpecifiedColor());
	
	FVector2D Distance = FVector2D(10,10);
	FVector2D Start = Offset / Distance;
	FVector2D RenderOffset = FVector2D(FMath::Fractional(Start.X) * Distance.X, FMath::Fractional(Start.Y) * Distance.Y);
	int GridOffsetX = FMath::FloorToInt(Start.X) * FMath::RoundToInt(Distance.X);
	int GridOffsetY = FMath::FloorToInt(Start.Y) * FMath::RoundToInt(Distance.Y);
	Distance *= Zoom;
	Start *= Zoom;
	RenderOffset *= Zoom;
	for (float x = 0; x <= AllottedGeometry.GetLocalSize().X; x += Distance.X) {
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(x + RenderOffset.X, 0), FVector2D(x + RenderOffset.X, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, GridColor, true, ((FMath::RoundToInt(x/Zoom) - GridOffsetX) % 100 == 0) ? 1.0 : 0.05);
	}
	for (float y = 0; y <= AllottedGeometry.GetLocalSize().Y; y += Distance.Y) {
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(0, y + RenderOffset.Y), FVector2D(AllottedGeometry.GetLocalSize().X, y + RenderOffset.Y)}, ESlateDrawEffect::None, GridColor, true, ((FMath::RoundToInt(y/Zoom) - GridOffsetY) % 100 == 0) ? 1.0 : 0.05);
	}

	// Draw Pin Connections
	TMap<TSharedRef<SFINScriptPinViewer>, FVector2D> ConnectionLocations; 
	TMap<TSharedPtr<FFINScriptPin>, TSharedRef<SFINScriptPinViewer>> PinMap;
	
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFINScriptNodeViewer> Node = Children[i];
		for (int j = 0; j < Node->GetPinWidgets().Num(); ++j) {
			TSharedRef<SFINScriptPinViewer> Pin = Node->GetPinWidgets()[j];
			ConnectionLocations.Add(Pin, Pin->GetConnectionPoint());
			PinMap.Add(Pin->GetPin(), Pin);
		}
	}

	TSet<TPair<TSharedPtr<FFINScriptPin>, TSharedPtr<FFINScriptPin>>> DrawnPins;
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFINScriptNodeViewer> Node = Children[i];
		for (const TSharedRef<SFINScriptPinViewer>& Pin : Node->GetPinWidgets()) {
			for (const TSharedPtr<FFINScriptPin>& ConnectionPin : Pin->GetPin()->GetConnections()) {
				if (!DrawnPins.Contains(TPair<TSharedPtr<FFINScriptPin>, TSharedPtr<FFINScriptPin>>(Pin->GetPin(), ConnectionPin)) && !DrawnPins.Contains(TPair<TSharedPtr<FFINScriptPin>, TSharedPtr<FFINScriptPin>>(ConnectionPin, Pin->GetPin()))) {
					DrawnPins.Add(TPair<TSharedPtr<FFINScriptPin>, TSharedPtr<FFINScriptPin>>(Pin->GetPin(), ConnectionPin));
					ConnectionDrawer->DrawConnection(Pin, NodeToChild[ConnectionPin->ParentNode]->GetPinWidget(ConnectionPin), SharedThis(this), AllottedGeometry, OutDrawElements, LayerId+1);
				}
			}
		}
	}

	if (bIsPinDrag) {
		TSharedRef<SFINScriptPinViewer> PinWidget = NodeToChild[PinDragStart->ParentNode]->GetPinWidget(PinDragStart);
		FVector2D StartLoc = GetCachedGeometry().AbsoluteToLocal(PinWidget->GetConnectionPoint());
		FVector2D EndLoc = PinDragEnd;
		if (PinDragStart->GetPinType() & FIVS_PIN_INPUT) {
			EndLoc = StartLoc;
			StartLoc = PinDragEnd;
		}
		ConnectionDrawer->DrawConnection(StartLoc, EndLoc, PinWidget->GetPinColor().GetSpecifiedColor(), SharedThis(this), AllottedGeometry, OutDrawElements, LayerId+100);
	}

	if (bIsSelectionDrag) {
		FVector2D SelectStart = GetCachedGeometry().AbsoluteToLocal(SelectionDragStart);
		FVector2D SelectEnd = GetCachedGeometry().AbsoluteToLocal(SelectionDragEnd);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId+200, AllottedGeometry.ToPaintGeometry(SelectStart, SelectEnd - SelectStart), &SelectionBrush, ESlateDrawEffect::None, FLinearColor(1,1,1,0.1));
	}
	
	return ret;
}

FReply SFINScriptGraphViewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (!MouseEvent.GetModifierKeys().IsShiftDown() && !SelectedNodes.Contains(NodeUnderMouse)) DeselectAll();
	if (NodeUnderMouse) {
		if (PinUnderMouse.IsValid() && !MouseEvent.GetModifierKeys().IsControlDown()) {
			bIsPinDrag = true;
			PinDragStart = PinUnderMouse;
		} else {
			bIsNodeDrag = true;
			NodeDragStart = MouseEvent.GetScreenSpacePosition();
			Select(NodeUnderMouse);
			NodeDragPosStart.Empty();
			for (UFINScriptNode* Node : SelectedNodes) {
				NodeDragPosStart.Add(Node->Pos);
			}
		}
		return FReply::Handled();
	}
	if (ConnectionDrawer->ConnectionUnderMouse.Key.IsValid() && MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		TPair<TSharedPtr<SFINScriptPinViewer>, TSharedPtr<SFINScriptPinViewer>> Connection = ConnectionDrawer->ConnectionUnderMouse;
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
		return FReply::Handled();
	}
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		bIsSelectionDrag = true;
		SelectionDragStart = MouseEvent.GetScreenSpacePosition();
		return FReply::Handled();
	}
	bIsGraphDrag = true;
	return SPanel::OnMouseButtonDown(MyGeometry, MouseEvent);
}
FReply SFINScriptGraphViewer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (bIsPinDrag) {
		bIsPinDrag = false;
		if (PinUnderMouse.IsValid()) {
			PinDragStart->AddConnection(PinUnderMouse.ToSharedRef());
		}
	} else if (bIsNodeDrag) {
		bIsNodeDrag = false;
		return FReply::Handled();
	} else if (bIsSelectionDrag) {
		bIsSelectionDrag = false;
	} else if (bIsGraphDrag) {
		bIsGraphDrag = false;
	}
	return SPanel::OnMouseButtonUp(MyGeometry, MouseEvent);
}

FReply SFINScriptGraphViewer::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && ConnectionDrawer.IsValid() && ConnectionDrawer->ConnectionUnderMouse.Key) {
		TPair<TSharedPtr<SFINScriptPinViewer>, TSharedPtr<SFINScriptPinViewer>> Connection = ConnectionDrawer->ConnectionUnderMouse;
		TSharedPtr<FFINScriptPin> Pin1 = Connection.Key->GetPin();
		TSharedPtr<FFINScriptPin> Pin2 = Connection.Value->GetPin();
		UFINScriptRerouteNode* Node = NewObject<UFINScriptRerouteNode>();
		Node->Pos = LocalToGraph(GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition())) - FVector2D(10, 10);
		Pin1->RemoveConnection(Connection.Value->GetPin());
		Pin1->AddConnection(Node->GetNodePins()[0]);
		Pin2->AddConnection(Node->GetNodePins()[0]);
		Graph->AddNode(Node);
		return FReply::Handled();
	}
	return SPanel::OnMouseButtonDoubleClick(InMyGeometry, InMouseEvent);
}

FReply SFINScriptGraphViewer::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(MyGeometry, ArrangedChildren);
	int ChildUnderMouseIndex = FindChildUnderMouse(ArrangedChildren, MouseEvent);
	if (ChildUnderMouseIndex >= 0) {
		NodeUnderMouse = Children[ChildUnderMouseIndex]->GetNode();
		PinUnderMouse = NodeToChild[NodeUnderMouse]->GetPinUnderMouse();
	} else {
		NodeUnderMouse = nullptr;
	}
	if (ConnectionDrawer) ConnectionDrawer->LastMousePosition = MouseEvent.GetScreenSpacePosition();
	if (bIsPinDrag) {
		PinDragEnd = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	if (bIsNodeDrag) {
		for (int i = 0; i < SelectedNodes.Num(); ++i) {
			bool bSnapToGrid = !MouseEvent.GetModifierKeys().IsControlDown();
			FVector2D MoveOffset = LocalToGraph(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())) - LocalToGraph(MyGeometry.AbsoluteToLocal(NodeDragStart));
			if (bSnapToGrid) MoveOffset = FVector2D(FMath::RoundToFloat(MoveOffset.X/10.0)*10.0, FMath::RoundToFloat(MoveOffset.Y/10.0)*10.0);
			UFINScriptNode* Node = SelectedNodes[i];
			Node->Pos = NodeDragPosStart[i] + MoveOffset;
			if (bSnapToGrid && SelectedNodes.Num() == 1 && !MouseEvent.GetModifierKeys().IsShiftDown()) {
				FVector2D NPos = Node->Pos / 10.0;
				Node->Pos = FVector2D(FMath::RoundToFloat(NPos.X), FMath::RoundToFloat(NPos.Y))*10.0;
			}
		}
		return FReply::Handled();
	}
	if (bIsSelectionDrag) {
		if (!FSlateApplication::Get().GetModifierKeys().IsShiftDown()) DeselectAll();
		SelectionDragEnd = MouseEvent.GetScreenSpacePosition();
		FVector2D LocalStart = GetCachedGeometry().AbsoluteToLocal(SelectionDragStart);
		FVector2D LocalEnd = GetCachedGeometry().AbsoluteToLocal(SelectionDragEnd);
		FSlateRect SelectionRect = FSlateRect(FVector2D(FMath::Min(LocalStart.X, LocalEnd.X), FMath::Min(LocalStart.Y, LocalEnd.Y)), FVector2D(FMath::Max(LocalStart.X, LocalEnd.X), FMath::Max(LocalStart.Y, LocalEnd.Y)));
		for (UFINScriptNode* Node : Graph->GetNodes()) {
			TSharedRef<SFINScriptNodeViewer> NodeW = NodeToChild[Node];
			FSlateRect NodeRect = FSlateRect(GetCachedGeometry().AbsoluteToLocal(NodeW->GetCachedGeometry().GetAbsolutePosition()), GetCachedGeometry().AbsoluteToLocal(NodeW->GetCachedGeometry().GetAbsolutePositionAtCoordinates(FVector2D(1,1))));
			if (Node && FSlateRect::DoRectanglesIntersect(NodeRect, SelectionRect)) {
				Select(Node);
			}
		}
		return FReply::Handled();
	}
	if (bIsGraphDrag) {
		Offset += LocalToGraph(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())) - LocalToGraph(MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition()));
		return FReply::Handled();
	}
	return SPanel::OnMouseMove(MyGeometry, MouseEvent);
}

FReply SFINScriptGraphViewer::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	FVector2D StartPos = LocalToGraph(GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	Zoom += MouseEvent.GetWheelDelta() / 10.0;
	if (Zoom < 0.1) Zoom = 0.1;
	if (Zoom > 10) Zoom = 10;
	FVector2D EndPos = LocalToGraph(GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()));
	FVector2D Off = StartPos - EndPos;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%f %f %f %f %f %f"), StartPos.X, StartPos.Y, EndPos.X, EndPos.Y, Off.X, Off.Y));
	Offset -= Off;
	return FReply::Handled();
}

FReply SFINScriptGraphViewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Delete) {
		if (SelectedNodes.Num() > 0) {
			for (UFINScriptNode* Node : SelectedNodes) {
				Graph->RemoveNode(Node);
			}
		} else if (ConnectionDrawer && ConnectionDrawer->ConnectionUnderMouse.Key) {
			ConnectionDrawer->ConnectionUnderMouse.Key->GetPin()->RemoveConnection(ConnectionDrawer->ConnectionUnderMouse.Value->GetPin());
		}
	}
	return SPanel::OnKeyDown(MyGeometry, InKeyEvent);
}
FReply SFINScriptGraphViewer::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	return SPanel::OnKeyUp(MyGeometry, InKeyEvent);
}

bool SFINScriptGraphViewer::IsInteractable() const {
	return true;
}

bool SFINScriptGraphViewer::SupportsKeyboardFocus() const {
	return true;
}

FChildren* SFINScriptGraphViewer::GetChildren() {
	return &Children;
}

void SFINScriptGraphViewer::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	for (int32 NodeIndex = 0; NodeIndex < Children.Num(); ++NodeIndex) {
		const TSharedRef<SFINScriptNodeViewer>& Node = Children[NodeIndex];
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Node, Node->GetPosition() + Offset, Node->GetDesiredSize(), Zoom));
	}
}

void SFINScriptGraphViewer::OnNodeChanged(int change, UFINScriptNode* Node) {
	if (change == 0) {
		CreateNodeAsChild(Node);
	} else {
		TSharedRef<SFINScriptNodeViewer>* Viewer = NodeToChild.Find(Node);
		if (Viewer) Children.Remove(*Viewer);
	}
}

void SFINScriptGraphViewer::SetGraph(UFINScriptGraph* NewGraph) {
	if (Graph) {
		Graph->OnNodeChanged.Remove(OnNodeChangedHandle);
		Children.Empty();
		NodeToChild.Empty();
	}
	
	Graph = NewGraph;

	if (Graph) {
		OnNodeChangedHandle = Graph->OnNodeChanged.AddRaw(this, &SFINScriptGraphViewer::OnNodeChanged);
		
		// Generate Nodes Children
		for (UFINScriptNode* Node : Graph->GetNodes()) {
			CreateNodeAsChild(Node);
		}
	}
}

void SFINScriptGraphViewer::CreateNodeAsChild(UFINScriptNode* Node) {
	TSharedRef<SFINScriptNodeViewer> Child = SNew(SFINScriptNodeViewer)
        .Node(Node);
	Children.Add(Child);
	NodeToChild.Add(Node, Child);
}

void SFINScriptGraphViewer::Select(UFINScriptNode* Node) {
	if (SelectedNodes.Contains(Node)) return;
	SelectedNodes.Add(Node);
	NodeToChild[Node]->bSelected = true;
}

void SFINScriptGraphViewer::Deselect(UFINScriptNode* Node) {
	if (SelectedNodes.Remove(Node)) {
		NodeToChild[Node]->bSelected = false;
	}
}

void SFINScriptGraphViewer::DeselectAll() {
	TArray<UFINScriptNode*> Nodes = SelectedNodes;
	for (UFINScriptNode* Node : Nodes) {
		Deselect(Node);
	}
}

FVector2D SFINScriptGraphViewer::LocalToGraph(const FVector2D Local) {
	return (Local / Zoom) - Offset;
}
