﻿#include "Editor/FIVSEdGraphViewer.h"

#include "FicsItVisualScriptModule.h"
#include "Editor/FIVSEdActionSelection.h"
#include "Editor/FIVSEdNodeViewer.h"
#include "Script/FIVSGraph.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "FIVSEdGraphViewerModule"

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

	FVector2D StartLoc = Graph->GraphToLocal(GetAndCachePinPosition(Pin1, Graph, AllottedGeometry));
	FVector2D EndLoc = Graph->GraphToLocal(GetAndCachePinPosition(Pin2, Graph, AllottedGeometry));

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
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), {Start, End}, ESlateDrawEffect::None, ConnectionColor, true, 2 * Graph->GetZoom());
}

FVector2D FFIVSEdConnectionDrawer::GetAndCachePinPosition(const TSharedRef<SFIVSEdPinViewer>& Pin, const TSharedRef<const SFIVSEdGraphViewer>& Graph, const FGeometry& AllottedGeometry) {
	if (Graph->IsNodeCulled(Pin->GetNodeViewer().ToSharedRef(), AllottedGeometry)) {
		FVector2D* position = PinPositions.Find(Pin);
		if (position) {
			return *position;
		} else {
			return Pin->GetNodeViewer()->GetPosition();
		}
	} else {
		FVector2D position = Graph->LocalToGraph(Graph->GetCachedGeometry().AbsoluteToLocal(Pin->GetConnectionPoint()));
		PinPositions.Add(Pin, position);
		return position;
	}
}

SFIVSEdGraphViewer::FCommands::FCommands() : TCommands<FCommands>( TEXT("GraphView"), LOCTEXT("Context", "Graph View"), NAME_None, FAppStyle::GetAppStyleSetName()) {}

void SFIVSEdGraphViewer::FCommands::RegisterCommands() {
	UI_COMMAND(CopySelection, "Copy Selection", "Copies the currently selected nodes and connections to the clipboard.", EUserInterfaceActionType::Button, FInputChord(EKeys::C, EModifierKey::Control))
	UI_COMMAND(PasteSelection, "Paste", "Pastes the Clipboard as Nodes into the graph at the current cursor location.", EUserInterfaceActionType::Button, FInputChord(EKeys::V, EModifierKey::Control))
	UI_COMMAND(DeleteSelection, "Delete Selection", "Deletes the currently selected nodes.", EUserInterfaceActionType::Button, FInputChord(EKeys::Delete))
	UI_COMMAND(CenterGraph, "Center Graph", "Centers the graph view back to the origin.", EUserInterfaceActionType::Button, FInputChord(EKeys::Home))
}

void SFIVSEdGraphViewer::Construct(const FArguments& InArgs) {
	Style = InArgs._Style;
	SetGraph(InArgs._Graph);
	SelectionChanged = InArgs._OnSelectionChanged;
	SelectionManager.OnSelectionChanged.BindRaw(this, &SFIVSEdGraphViewer::OnSelectionChanged);

	FCommands::Register();

	CommandList = MakeShared<FUICommandList>();

	CommandList->MapAction(FCommands::Get().CopySelection, FExecuteAction::CreateLambda([this]() {
		FString Serialized = UFIVSSerailizationUtils::FIVS_SerializePartial(SelectionManager.GetSelection(), true);

		/*TArray<uint8> Data;
		FMemoryWriter Archive(Data);
		FFIVSGraphProxy Proxy(Archive);
		//FJsonArchiveOutputFormatter Formatter(Proxy);
		//FStructuredArchive StructuredArchive(Formatter);
		//FStructuredArchiveSlot Slot = StructuredArchive.Open();
		//Graph->Serialize(Slot.EnterRecord());
		//StructuredArchive.Close();
		Graph->Serialize(Proxy);
		Serialized = UTF8_TO_TCHAR(Data.GetData());*/

		FPlatformApplicationMisc::ClipboardCopy(*Serialized);
	}));
	CommandList->MapAction(FCommands::Get().PasteSelection, FExecuteAction::CreateLambda([this]() {
		FString Paste;
		FPlatformApplicationMisc::ClipboardPaste(Paste);

		TArray<UFIVSNode*> Nodes = UFIVSSerailizationUtils::FIVS_DeserializeGraph(Graph, Paste, true);
		UFIVSSerailizationUtils::FIVS_AdjustNodesOffset(Nodes, LocalToGraph(GetCachedGeometry().AbsoluteToLocal(FSlateApplication::Get().GetCursorPos())), true);
		SelectionManager.SetSelection(Nodes);
	}));
	CommandList->MapAction(FCommands::Get().DeleteSelection, FExecuteAction::CreateLambda([this]() {
		if (SelectionManager.GetSelection().Num() > 0) {
			TArray<UFIVSNode*> Selection = SelectionManager.GetSelection();
			for (UFIVSNode* Node : Selection) {
				Graph->RemoveNode(Node);
			}
		} else if (ConnectionDrawer && ConnectionDrawer->ConnectionUnderMouse.Key) {
			ConnectionDrawer->ConnectionUnderMouse.Key->GetPin()->RemoveConnection(ConnectionDrawer->ConnectionUnderMouse.Value->GetPin());
		}
	}));
	CommandList->MapAction(FCommands::Get().CenterGraph, FExecuteAction::CreateLambda([this]() {
		Offset = FVector2D(0, 0);
	}));
}

SFIVSEdGraphViewer::SFIVSEdGraphViewer() : Children(this) {
	bHasRelativeLayoutScale = true;
	ConnectionDrawer = MakeShared<FFIVSEdConnectionDrawer>();
	SetCanTick(true);
}

SFIVSEdGraphViewer::~SFIVSEdGraphViewer() {
	if (Graph) Graph->OnNodeChanged.RemoveAll(this);
}

FVector2D SFIVSEdGraphViewer::ComputeDesiredSize(float) const {
	return FVector2D(160.0f, 120.0f);
}

void SFIVSEdGraphViewer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SPanel::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	auto dragDropContent = FSlateApplication::Get().GetDragDroppingContent();
	if (dragDropContent && dragDropContent->IsOfType<FFIVSEdGraphSelectionDragDrop>()) {
		auto content = StaticCastSharedPtr<FFIVSEdGraphSelectionDragDrop>(dragDropContent);
		if (&*content->GetGraphViewer() == this) {
			FVector2D min = AllottedGeometry.GetAbsolutePositionAtCoordinates({0.1,0.1});
			FVector2D max = AllottedGeometry.GetAbsolutePositionAtCoordinates({0.9,0.9});
			FVector2D pos = FSlateApplication::Get().GetCursorPos();
			double deltaTime = FSlateApplication::Get().GetDeltaTime();
			FVector2D offset = FVector2D::Zero();
			if (pos.X < min.X) offset.X += (min.X - pos.X) * deltaTime;
			if (pos.X > max.X) offset.X += (max.X - pos.X) * deltaTime;
			if (pos.Y < min.Y) offset.Y += (min.Y - pos.Y) * deltaTime;
			if (pos.Y > max.Y) offset.Y += (max.Y - pos.Y) * deltaTime;
			Offset += offset;
		}
	}
}

int32 SFIVSEdGraphViewer::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	OutDrawElements.PushClip(GetClipRect(AllottedGeometry));
	
	ConnectionDrawer->Reset();

	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), &Style->Background, ESlateDrawEffect::None, Style->Background.GetTint(InWidgetStyle));

	DrawGrid(LayerId, AllottedGeometry, OutDrawElements, InWidgetStyle);

	int ret = SPanel::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId+25, InWidgetStyle, bParentEnabled);

	DrawConnections(LayerId+1, AllottedGeometry, OutDrawElements, InWidgetStyle);
	DrawNewConnection(LayerId+100, AllottedGeometry, OutDrawElements, InWidgetStyle);
	DrawSelectionBox(LayerId+200, AllottedGeometry, OutDrawElements, InWidgetStyle);

	OutDrawElements.PopClip();
	
	return ret;
}

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
	} else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::RightMouseButton);
	} else if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
		return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

FReply SFIVSEdGraphViewer::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton) {
		CreateActionSelectionMenu(*MouseEvent.GetEventPath(), MouseEvent.GetScreenSpacePosition(), [this](auto){}, FFINScriptNodeCreationContext(Graph, LocalToGraph(GetCachedGeometry().AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())), nullptr));

		return FReply::Handled();
	}

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
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

class FFIVSGraphProxy : public FArchiveProxy {
public:
	FFIVSGraphProxy(FArchive& Inner) : FArchiveProxy(Inner) {}

	virtual FArchive& operator<<(UObject*& Value) override {
		if (!IsValid(Value)) return *this;

		if (!Objects.IsEmpty()) {
			int32 index = Objects.Find(Value);
			if (index != INDEX_NONE) {
				InnerArchive << index;
				return *this;
			} else if (!Objects.Contains(Value->GetOuter())) {
				return *this;
			}
		}

		Objects.Add(Value);
		Value->Serialize(*this);

		return *this;
	}

	virtual FArchive& operator<<(FObjectPtr& Value) override {
		UObject* Ptr = Value.Get();
		*this << Ptr;
		return *this;
	}

	virtual FArchive& operator<<(FLazyObjectPtr& Value) override {
		UObject* Ptr = Value.Get();
		*this << Ptr;
		return *this;
	}

	virtual FArchive& operator<<(FSoftObjectPath& Value) override
	{
		UObject* Ptr = Value.ResolveObject();
		*this << Ptr;
		return *this;
	}

	virtual FArchive& operator<<(FSoftObjectPtr& Value) override {
		UObject* Ptr = Value.Get();
		*this << Ptr;
		return *this;
	}

	virtual FArchive& operator<<(FWeakObjectPtr& Value) override {
		UObject* Ptr = Value.Get();
		*this << Ptr;
		return *this;
	}

private:
	TArray<UObject*> Objects;
};

UE_DISABLE_OPTIMIZATION_SHIP
FReply SFIVSEdGraphViewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (CommandList->ProcessCommandBindings(InKeyEvent)) {
		return FReply::Handled();
	}

	return SPanel::OnKeyDown(MyGeometry, InKeyEvent);
}
UE_ENABLE_OPTIMIZATION_SHIP

FReply SFIVSEdGraphViewer::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	return SPanel::OnKeyUp(MyGeometry, InKeyEvent);
}

FReply SFIVSEdGraphViewer::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton)) {
		return FReply::Handled()
		.BeginDragDrop(MakeShared<FFIVSEdGraphPanDragDrop>(SharedThis(this)));
	}

	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton)) {
		return FReply::Handled()
		.BeginDragDrop(MakeShared<FFIVSEdGraphSelectionDragDrop>(
			SharedThis(this),
			LocalToGraph(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()))
		));
	}

	return FReply::Unhandled();
}

FReply SFIVSEdGraphViewer::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
	if (DragDropEvent.GetOperationAs<FFIVSEdPinConnectDragDrop>()) {
		DraggingPinsEndpoint = DragDropEvent.GetScreenSpacePosition();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

FReply SFIVSEdGraphViewer::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
	if (TSharedPtr<FFIVSEdPinConnectDragDrop> PinConnect = DragDropEvent.GetOperationAs<FFIVSEdPinConnectDragDrop>()) {
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
		ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Node, FLayoutGeometry(FSlateLayoutTransform(GetZoom(), GraphToLocal(Node->GetPosition())), Node->GetDesiredSize())));
	}
}

float SFIVSEdGraphViewer::GetRelativeLayoutScale(const int32 ChildIndex, float LayoutScaleMultiplier) const {
	return GetZoom();
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
	
    TSharedRef<SFIVSEdActionSelection> Select = SNew(SFIVSEdActionSelection, Context.Pin ? FFIVSFullPinType(Context.Pin->GetPinType(), Context.Pin->GetPinDataType()) : FFIVSFullPinType(FIVS_PIN_DATA_INPUT & FIVS_PIN_EXEC_OUTPUT, FFIVSPinDataType(FIR_NIL)))
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

FVector2D SFIVSEdGraphViewer::LocalToGraph(const FVector2D& Local) const {
	return (Local / GetZoom()) - Offset;
}

FVector2D SFIVSEdGraphViewer::GraphToLocal(const FVector2D& InGraph) const {
	return (InGraph + Offset) * GetZoom();
}

void SFIVSEdGraphViewer::SetGraph(UFIVSGraph* NewGraph) {
	if (IsValid(Graph)) {
		Graph->OnNodeChanged.RemoveAll(this);
		Children.Empty();
		NodeToChild.Empty();
	}

	Graph = NewGraph;

	if (IsValid(Graph)) {
		Graph->OnNodeChanged.AddRaw(this, &SFIVSEdGraphViewer::OnNodeChanged);

		// Generate Nodes Children
		for (UFIVSNode* Node : Graph->GetNodes()) {
			CreateNodeAsChild(Node);
		}
	}
}

void SFIVSEdGraphViewer::UpdateSelection(const FPointerEvent& Event) {
	if (!SelectionBoxHandler.IsActive()) return;

	FGeometry geometry = GetCachedGeometry();

	SelectionBoxHandler.UpdateDrag(LocalToGraph(geometry.AbsoluteToLocal(Event.GetScreenSpacePosition())));

	FBox2D graphBox = SelectionBoxHandler.GetRect();
	FSlateRect SelectionRect = FSlateRect(
		geometry.LocalToAbsolute(GraphToLocal(graphBox.Min)),
		geometry.LocalToAbsolute(GraphToLocal(graphBox.Max)));

	if (Graph) {
		TArray<UFIVSNode*> SelectedNodes;
		for (UFIVSNode* Node : Graph->GetNodes()) {
			TSharedRef<SFIVSEdNodeViewer> NodeW = NodeToChild[Node];

			FGeometry NodeGeo = NodeW->GetCachedGeometry();
			FSlateRect NodeRect = FSlateRect(NodeGeo.GetAbsolutePosition(), NodeGeo.GetAbsolutePositionAtCoordinates(FVector2D(1,1)));

			if (Node && FSlateRect::DoRectanglesIntersect(NodeRect, SelectionRect)) {
				SelectedNodes.Add(Node);
			}
		}
		SelectionManager.Select(SelectedNodes, Event);
	}
}

void SFIVSEdGraphViewer::CreateNodeAsChild(UFIVSNode* Node) {
	check(IsValid(Node));

	TSharedRef<SFIVSEdNodeViewer> Child = Node->CreateNodeViewer(SharedThis(this), &Style->NodeStyle);
	Children.Add(Child);
	NodeToChild.Add(Node, Child);
}

void SFIVSEdGraphViewer::OnNodeChanged(EFIVSNodeChange change, UFIVSNode* Node) {
	check(IsValid(Node));

	switch (change) {
	case FIVS_Node_Added:
		CreateNodeAsChild(Node);
		break;
	case FIVS_Node_Removed: {
		SelectionManager.SetSelected(Node, false);
		TSharedRef<SFIVSEdNodeViewer>* Viewer = NodeToChild.Find(Node);
		if (Viewer) Children.Remove(*Viewer);
		NodeToChild.Remove(Node);
		break;
	} default: ;
	}
}

void SFIVSEdGraphViewer::OnSelectionChanged(UFIVSNode* InNode, bool bIsSelected) {
	check(IsValid(InNode));

	NodeToChild[InNode]->bSelected = bIsSelected;
	SelectionChanged.ExecuteIfBound(InNode, bIsSelected);
}

FSlateClippingZone SFIVSEdGraphViewer::GetClipRect(const FGeometry& AllottedGeometry) const {
	return FSlateClippingZone(AllottedGeometry);
}

bool SFIVSEdGraphViewer::IsNodeCulled(const TSharedRef<SFIVSEdNodeViewer>& Node, const FGeometry& AllottedGeometry) const {
	FBox2D NodeBox(Node->GetPosition(), Node->GetPosition() + Node->GetDesiredSize());
	FBox2D GraphBox(LocalToGraph({0.0,0.0}), LocalToGraph(AllottedGeometry.GetLocalSize()));

	return !NodeBox.Intersect(GraphBox);
}

void SFIVSEdGraphViewer::DrawGrid(uint32 LayerId, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, const FWidgetStyle& InWidgetStyle) const {
	FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	FVector2D GraphMin = LocalToGraph({0.0,0.0});
	FVector2D GraphMax = LocalToGraph(LocalSize);
	FVector2D VisibleGraphSize = GraphMax - GraphMin;

	double minGraphStep = 10;
	double base = 5;
	double majorStep = 10;
	double GraphStep;
	GraphStep = FMath::Pow(base, FMath::Floor(FMath::LogX(base, VisibleGraphSize.X / minGraphStep)));
	GraphStep = FMath::Min(GraphStep, FMath::Pow(base, FMath::Floor(FMath::LogX(base, VisibleGraphSize.Y / minGraphStep))));

	for (float x = FMath::RoundUpToClosestMultiple(GraphMin.X, GraphStep); x <= GraphMax.X; x += GraphStep) {
		bool bIsMajor = FMath::IsNearlyZero(FMath::Fmod(x, GraphStep * majorStep));
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			{GraphToLocal({x, GraphMin.Y}), GraphToLocal({x, GraphMax.Y})},
			ESlateDrawEffect::None,
			(bIsMajor ? Style->GridMajorColor : Style->GridMinorColor).GetSpecifiedColor(),
			true,
			bIsMajor ? 1.0 : 0.05);
	}
	for (float y = FMath::RoundUpToClosestMultiple(GraphMin.Y, GraphStep); y <= GraphMax.Y; y += GraphStep) {
		bool bIsMajor = FMath::IsNearlyZero(FMath::Fmod(y, GraphStep * majorStep));
		FSlateDrawElement::MakeLines(OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			{GraphToLocal({GraphMin.X, y}), GraphToLocal({GraphMax.X, y})},
			ESlateDrawEffect::None,
			(bIsMajor ? Style->GridMajorColor : Style->GridMinorColor).GetSpecifiedColor(),
			true,
			bIsMajor ? 1.0 : 0.05);
	}
}

void SFIVSEdGraphViewer::DrawConnections(uint32 LayerId, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, const FWidgetStyle& InWidgetStyle) const {
	TSet<TPair<UFIVSPin*, UFIVSPin*>> DrawnPins;
	for (int i = 0; i < Children.Num(); ++i) {
		TSharedRef<SFIVSEdNodeViewer> Node = Children[i];
		for (const TSharedRef<SFIVSEdPinViewer>& Pin : Node->GetPinWidgets()) {
			for (UFIVSPin* ConnectionPin : Pin->GetPin()->GetConnections()) {
				if (!DrawnPins.Contains({Pin->GetPin(), ConnectionPin}) && !DrawnPins.Contains({ConnectionPin, Pin->GetPin()})) {
					DrawnPins.Add(TPair<UFIVSPin*, UFIVSPin*>(Pin->GetPin(), ConnectionPin));
					ConnectionDrawer->DrawConnection(Pin, NodeToChild[ConnectionPin->ParentNode]->GetPinWidget(ConnectionPin), SharedThis(this), AllottedGeometry, OutDrawElements, LayerId);
				}
			}
		}
	}
}

void SFIVSEdGraphViewer::DrawNewConnection(uint32 LayerId, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, const FWidgetStyle& InWidgetStyle) const {
	for (TSharedRef<SFIVSEdPinViewer> DraggingPin : DraggingPins) {
		FVector2D StartLoc = GetCachedGeometry().AbsoluteToLocal(DraggingPin->GetConnectionPoint());
		FVector2D EndLoc = GetCachedGeometry().AbsoluteToLocal(DraggingPinsEndpoint);
		if (DraggingPin->GetPin()->GetPinType() & FIVS_PIN_INPUT) {
			Swap(EndLoc, StartLoc);
		}
		ConnectionDrawer->DrawConnection(StartLoc, EndLoc, DraggingPin->GetPinColor().GetSpecifiedColor(), SharedThis(this), AllottedGeometry, OutDrawElements, LayerId);
	}
}

void SFIVSEdGraphViewer::DrawSelectionBox(uint32 LayerId, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, const FWidgetStyle& InWidgetStyle) const {
	if (SelectionBoxHandler.IsActive()) {
		FBox2D box = SelectionBoxHandler.GetRect();
		FVector2D min = GraphToLocal(box.Min);
		FVector2D max = GraphToLocal(box.Max);
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(max - min, FSlateLayoutTransform(min)), &Style->SelectionBox, ESlateDrawEffect::None, FLinearColor(1,1,1,0.1));
	}
}

FFIVSEdGraphPanDragDrop::FFIVSEdGraphPanDragDrop(TSharedRef<SFIVSEdGraphViewer> GraphViewer) : GraphViewer(GraphViewer) {}

FCursorReply FFIVSEdGraphPanDragDrop::OnCursorQuery() {
	return FCursorReply::Cursor(EMouseCursor::Type::GrabHandClosed);
}

void FFIVSEdGraphPanDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	FGeometry geometry = GraphViewer->GetCachedGeometry();
	FVector2D screenPos = DragDropEvent.GetScreenSpacePosition();
	FVector2D graphPos = GraphViewer->LocalToGraph(geometry.AbsoluteToLocal(screenPos));
	if (prevScreenPos) {
		FVector2D prevGraphPos = GraphViewer->LocalToGraph(geometry.AbsoluteToLocal(*prevScreenPos));
		GraphViewer->Offset += graphPos - prevGraphPos;
	}
	prevScreenPos = screenPos;
	GraphViewer->Invalidate(EInvalidateWidgetReason::Paint);
	return;
	FVector2D newScreenPos = screenPos;
	FBox2D absoluteWrapRect = FBox2D(geometry.GetAbsolutePositionAtCoordinates(FVector2D::Zero()), geometry.GetAbsolutePositionAtCoordinates(FVector2D::One()));
	if (newScreenPos.X < absoluteWrapRect.Min.X) newScreenPos.X += absoluteWrapRect.GetSize().X;
	else if (newScreenPos.X > absoluteWrapRect.Max.X) newScreenPos.X -= absoluteWrapRect.GetSize().X;
	if (newScreenPos.Y < absoluteWrapRect.Min.Y) newScreenPos.Y += absoluteWrapRect.GetSize().Y;
	else if (newScreenPos.Y > absoluteWrapRect.Max.Y) newScreenPos.Y -= absoluteWrapRect.GetSize().Y;
	prevScreenPos = newScreenPos;
	if (newScreenPos != screenPos) {
		FSlateApplication::Get().SetCursorPos(newScreenPos);
	}
}

FFIVSEdGraphSelectionDragDrop::FFIVSEdGraphSelectionDragDrop(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, FVector2D StartPos): GraphViewer(GraphViewer) {
	GraphViewer->SelectionBoxHandler.BeginDrag(StartPos);
}

void FFIVSEdGraphSelectionDragDrop::OnDragged(const FDragDropEvent& DragDropEvent) {
	GraphViewer->UpdateSelection(DragDropEvent);
}

void FFIVSEdGraphSelectionDragDrop::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) {
	GraphViewer->SelectionBoxHandler.EndDrag();
}