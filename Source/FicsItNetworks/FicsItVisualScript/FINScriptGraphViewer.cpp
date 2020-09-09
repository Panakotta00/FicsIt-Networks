#include "FINScriptGraphViewer.h"

void SFINScriptGraphViewer::Construct(const FArguments& InArgs) {
	SetGraph(InArgs._Graph.Get());
}

SFINScriptGraphViewer::SFINScriptGraphViewer() : Children(this) {}

SFINScriptGraphViewer::~SFINScriptGraphViewer() {
	if (Graph) Graph->OnNodeChanged.Remove(OnNodeChangedHandle);
}

FVector2D SFINScriptGraphViewer::ComputeDesiredSize(float) const {
	return FVector2D(1000,1000);
}

int32 SFINScriptGraphViewer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	int ret = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+1, InWidgetStyle, bParentEnabled);

	// Draw Grid
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), &BackgroundBrush, ESlateDrawEffect::None, BackgroundBrush.TintColor.GetSpecifiedColor());
	
	FVector2D Distance = FVector2D(10,10);
	FVector2D Start = Offset / Distance;
	FVector2D RenderOffset = FVector2D(FMath::Fractional(Start.X) * Distance.X, FMath::Fractional(Start.Y) * Distance.Y);
	int GridOffsetX = FMath::FloorToInt(Start.X) * FMath::RoundToInt(Distance.X);
	int GridOffsetY = FMath::FloorToInt(Start.Y) * FMath::RoundToInt(Distance.Y);
	for (float x = 0; x <= AllottedGeometry.GetLocalSize().X; x += Distance.X) {
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(x + RenderOffset.X, 0), FVector2D(x + RenderOffset.X, AllottedGeometry.GetLocalSize().Y)}, ESlateDrawEffect::None, GridColor, true, ((FMath::RoundToInt(x) - GridOffsetX) % 100 == 0) ? 1.0 : 0.05);
	}
	for (float y = 0; y <= AllottedGeometry.GetLocalSize().Y; y += Distance.Y) {
		FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {FVector2D(0, y + RenderOffset.Y), FVector2D(AllottedGeometry.GetLocalSize().X, y + RenderOffset.Y)}, ESlateDrawEffect::None, GridColor, true, ((FMath::RoundToInt(y) - GridOffsetY) % 100 == 0) ? 1.0 : 0.05);
	}

	// Draw Pin Connections
	TMap<TSharedRef<SFINScriptPinViewer>, FVector2D> ConnectionLocations; 
	TMap<FFINScriptPin*, TSharedRef<SFINScriptPinViewer>> PinMap;
	
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFINScriptNodeViewer> Node = Children[i];
		for (int j = 0; j < Node->GetPinWidgets().Num(); ++j) {
			TSharedRef<SFINScriptPinViewer> Pin = Node->GetPinWidgets()[j];
			ConnectionLocations.Add(Pin, Pin->GetConnectionPoint());
			PinMap.Add(Pin->GetPin().Get(), Pin);
		}
	}

	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFINScriptNodeViewer> Node = Children[i];
		for (int j = 0; j < Node->GetPinWidgets().Num(); ++j) {
			TSharedRef<SFINScriptPinViewer> Pin = Node->GetPinWidgets()[j];
			if (Pin->GetPin()->PinType & FIVS_PIN_OUTPUT) {
				FVector2D StartLoc = GetCachedGeometry().AbsoluteToLocal(ConnectionLocations[Pin]);
				for (FFINScriptPin* ConnectionPin : Pin->GetPin()->GetConnections()) {
					TSharedRef<SFINScriptPinViewer> Connection = PinMap[ConnectionPin];
					FVector2D EndLoc = GetCachedGeometry().AbsoluteToLocal(ConnectionLocations[Connection]);
					FSlateDrawElement::MakeSpline(OutDrawElements, LayerId+100, AllottedGeometry.ToPaintGeometry(), StartLoc, FVector2D(300,0), EndLoc, FVector2D(300,0), 2, ESlateDrawEffect::None, Pin->GetPinColor().GetSpecifiedColor());
				}
			}
		}
	}

	if (bIsPinDrag) {
		TSharedRef<SFINScriptPinViewer> PinWidget = NodeToChild[PinDragStart->ParentNode]->GetPinWidget(PinDragStart);
		FVector2D StartLoc = GetCachedGeometry().AbsoluteToLocal(PinWidget->GetConnectionPoint());
		FVector2D EndLoc = PinDragEnd;
		if (PinDragStart->PinType & FIVS_PIN_INPUT) {
			EndLoc = StartLoc;
			StartLoc = PinDragEnd;
		}
		FSlateDrawElement::MakeSpline(OutDrawElements, LayerId+100, AllottedGeometry.ToPaintGeometry(), StartLoc, FVector2D(300,0), EndLoc, FVector2D(300,0), 2, ESlateDrawEffect::None, PinWidget->GetPinColor().GetSpecifiedColor());
	}
	
	return ret;
}

FReply SFINScriptGraphViewer::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (!MouseEvent.GetModifierKeys().IsShiftDown() && !SelectedNodes.Contains(NodeUnderMouse)) DeselectAll();
	if (NodeUnderMouse) {
		if (PinUnderMouse.IsValid()) {
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
	bIsGraphDrag = true;
	return SPanel::OnMouseButtonDown(MyGeometry, MouseEvent);
}
FReply SFINScriptGraphViewer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (bIsPinDrag) {
		bIsPinDrag = false;
		if (PinUnderMouse.IsValid()) {
			PinDragStart->AddConnection(PinUnderMouse.Get());
		}
	} else if (bIsNodeDrag) {
		bIsNodeDrag = false;
		return FReply::Handled();
	} else if (bIsGraphDrag) {
		bIsGraphDrag = false;
	}
	return SPanel::OnMouseButtonUp(MyGeometry, MouseEvent);
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
	//if (PinUnderMouse && GEngine) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, PinUnderMouse->Name);
	if (bIsPinDrag) {
		PinDragEnd = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	} else if (bIsNodeDrag) {
		for (int i = 0; i < SelectedNodes.Num(); ++i) {
			bool bSnapToGrid = !MouseEvent.GetModifierKeys().IsControlDown();
			FVector2D MoveOffset = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - MyGeometry.AbsoluteToLocal(NodeDragStart);
			if (bSnapToGrid) MoveOffset = FVector2D(FMath::RoundToFloat(MoveOffset.X/10.0)*10.0, FMath::RoundToFloat(MoveOffset.Y/10.0)*10.0);
			UFINScriptNode* Node = SelectedNodes[i];
			Node->Pos = NodeDragPosStart[i] + MoveOffset;
			if (bSnapToGrid && SelectedNodes.Num() == 1 && !MouseEvent.GetModifierKeys().IsShiftDown()) {
				FVector2D NPos = Node->Pos / 10.0;
				Node->Pos = FVector2D(FMath::RoundToFloat(NPos.X), FMath::RoundToFloat(NPos.Y))*10.0;
			}
		}
	} else if (bIsGraphDrag) {
		Offset += MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()) - MyGeometry.AbsoluteToLocal(MouseEvent.GetLastScreenSpacePosition());
	}
	return SPanel::OnMouseMove(MyGeometry, MouseEvent);
}
FReply SFINScriptGraphViewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
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
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Node, Node->GetPosition() + Offset, Node->GetDesiredSize(), 1));
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
