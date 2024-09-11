#pragma once

#include "CoreMinimal.h"
#include "FIVSEdActionSelection.h"
#include "FIVSEdGraphViewerStyle.h"
#include "Script/FIVSGraph.h"

struct FFIVSEdActionSelectionAction;
class SFIVSEdActionSelection;
class SFIVSEdPinViewer;
class SFIVSEdGraphViewer;
class SFIVSEdNodeViewer;

struct FFIVSEdConnectionDrawer {
public:
	float LastConnectionDistance;

public:
	TPair<TSharedPtr<SFIVSEdPinViewer>, TSharedPtr<SFIVSEdPinViewer>> ConnectionUnderMouse;
	FVector2D LastMousePosition;

	/**
	 * Resets the cache for the connection under mouse.
	 * Should get called in the top of the graphs onpaint.
	 */
	void Reset();
	
	/**
	* Draws a connection between two pins.
	* Checks if the mouse hovers over it, if so, sets connection under mouse.
	*/
	void DrawConnection(TSharedRef<SFIVSEdPinViewer> Pin1, TSharedRef<SFIVSEdPinViewer> Pin2, TSharedRef<const SFIVSEdGraphViewer> Graph, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId);

	/**
	* Draws a connection between two points.
	*
	* @param[in]	Start	the start point (outputs)
	* @param[in]	End		the end point (inputs)
	*/
	void DrawConnection(const FVector2D& Start, const FVector2D& End, const FLinearColor& ConnectionColor, TSharedRef<const SFIVSEdGraphViewer> Graph, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId);

	FVector2D GetAndCachePinPosition(const TSharedRef<SFIVSEdPinViewer>& Pin, const TSharedRef<const SFIVSEdGraphViewer>& Graph, const FGeometry& AllottedGeometry);

private:
	TMap<TWeakPtr<SFIVSEdPinViewer>, FVector2D> PinPositions;
};


class FFIVSEdSelectionManager {
public:
	typedef UFIVSNode* T;

	DECLARE_DELEGATE_TwoParams(FFIVSEdSelectionChanged, T, bool)
	
	FFIVSEdSelectionChanged OnSelectionChanged;
	
	void DeselectAll() {
		for (UFIVSNode* Node : Selection) OnSelectionChanged.ExecuteIfBound(Node, false);
		Selection.Empty();
	}
	
	void SetSelection(const TArray<T>& InSelection) {
		for (UFIVSNode* Node : Selection) OnSelectionChanged.ExecuteIfBound(Node, false);
		Selection = InSelection;
		for (UFIVSNode* Node : Selection) OnSelectionChanged.ExecuteIfBound(Node, true);
	}

	void SetSelected(T InObject, bool bInSelected) {
		int Index = Selection.Find(InObject);
		if (Index < 0) {
			if (bInSelected) {
				Selection.Add(InObject);
				OnSelectionChanged.ExecuteIfBound(InObject, true);
			}
		} else {
			if (!bInSelected) {
				Selection.RemoveAt(Index);
				OnSelectionChanged.ExecuteIfBound(InObject, false);
			}
		}
	}

	void Select(T InObject, const FInputEvent& InputEvent) {
		Select(TArray<T>{InObject}, InputEvent, true);
	}

	void Select(const TArray<T>& InObjects, const FInputEvent& InputEvent, bool bSingleSelect = false) {
		if (InputEvent.IsControlDown()) {
			for (T Object : InObjects) SetSelected(Object, true);
		} else if (InputEvent.IsShiftDown()) {
			for (T Object : InObjects) SetSelected(Object, false);
		} else {
			if (!bSingleSelect || !Selection.Contains(InObjects[0])) DeselectAll();
			for (T Object : InObjects) SetSelected(Object, true);
		}
	}
	
	const TArray<T>& GetSelection() { return Selection; }
	
private:
	TArray<T> Selection;
};

class FFIVSEdSelectionBoxHandler {
public:
	void BeginDrag(FVector2D InStartPos) {
		StartPos = EndPos = InStartPos;
		bIsActive = true;
	}

	void UpdateDrag(FVector2D InEndPos) {
		EndPos = InEndPos;
	}

	void EndDrag() {
		bIsActive = false;
	}

	bool IsActive() const {
		return bIsActive;
	}

	FBox2D GetRect() const {
		return FBox2D(
			FVector2D::Min(StartPos, EndPos),
			FVector2D::Max(StartPos, EndPos)
		);
	}

private:
	bool bIsActive = false;
	FVector2D StartPos;
	FVector2D EndPos;
};

DECLARE_DELEGATE_TwoParams(FFIVSEdSelectionChanged, UFIVSNode*, bool);

class SFIVSEdGraphViewer : public SPanel {
	class FCommands : public TCommands<FCommands> {
	public:
		FCommands();

		virtual void RegisterCommands() override;

		TSharedPtr<FUICommandInfo> CopySelection;
		TSharedPtr<FUICommandInfo> PasteSelection;
		TSharedPtr<FUICommandInfo> DeleteSelection;
		TSharedPtr<FUICommandInfo> CenterGraph;
	};

	SLATE_BEGIN_ARGS(SFIVSEdGraphViewer) :
		_Style(&FFIVSEdGraphViewerStyle::GetDefault()) {}
	SLATE_STYLE_ARGUMENT(FFIVSEdGraphViewerStyle, Style)
	SLATE_ARGUMENT(UFIVSGraph*, Graph)
	SLATE_EVENT(FFIVSEdSelectionChanged, OnSelectionChanged)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs );

private:
	const FFIVSEdGraphViewerStyle* Style = nullptr;
	UFIVSGraph* Graph = nullptr;
	FFIVSEdSelectionChanged SelectionChanged;
	TSharedPtr<FUICommandList> CommandList;

	TSlotlessChildren<SFIVSEdNodeViewer> Children;
	TMap<UFIVSNode*, TSharedRef<SFIVSEdNodeViewer>> NodeToChild;

	TArray<TSharedRef<SFIVSEdPinViewer>> DraggingPins;
	FVector2D DraggingPinsEndpoint;

	TSharedPtr<FFIVSEdConnectionDrawer> ConnectionDrawer;

	TSharedPtr<SFIVSEdActionSelection> ActiveActionSelection;

public:
	FFIVSEdSelectionManager SelectionManager;
	FFIVSEdSelectionBoxHandler SelectionBoxHandler;

	FVector2D Offset = FVector2D(0,0);
	float Zoom = 2;

	SFIVSEdGraphViewer();
	~SFIVSEdGraphViewer();

	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual float GetRelativeLayoutScale(const int32 ChildIndex, float LayoutScaleMultiplier) const override;
	// End SWidget

	void BeginDragPin(TSharedRef<SFIVSEdPinViewer> PinViewer) {
		DraggingPins.Add(PinViewer);
	}

	void EndDragPin(TSharedRef<SFIVSEdPinViewer> PinViewer) {
		DraggingPins.Remove(PinViewer);
	}

	float GetZoom() const {
		return exp(Zoom)/10;
	}

	/**
	 * Creates an action selection menu with the given context
	 */
	TSharedPtr<IMenu> CreateActionSelectionMenu(const FWidgetPath& Path, const FVector2D& Location, TFunction<void(const TSharedPtr<FFIVSEdActionSelectionAction>&)> OnExecute, const FFINScriptNodeCreationContext& Context);

	/**
	 * Converts the given local position to the position in the graph
	 */
	FVector2D LocalToGraph(const FVector2D& Local) const;

	/**
	 * Converts the given graph position to the local position
	 */
	FVector2D GraphToLocal(const FVector2D& InGraph) const;

	/**
	 * Sets the currently displayed Graph.
	 * Causes all child widgets to get regenerated.
	 *
	 * @param[in]	NewGraph	the new graph to display
	 */
	void SetGraph(UFIVSGraph* NewGraph);
	UFIVSGraph* GetGraph() { return Graph; }

	void UpdateSelection(const FPointerEvent& Event);

	FSlateClippingZone GetClipRect(const FGeometry& AllottedGeometry) const;
	bool IsNodeCulled(const TSharedRef<SFIVSEdNodeViewer>& Node, const FGeometry& AllottedGeometry) const;

private:
	/**
	 * Adds the node at the given index in the graph
	 * as widget to children.
	 *
	 * @pragma[in]	Node
	 */
	void CreateNodeAsChild(UFIVSNode* Node);

	void OnNodeChanged(EFIVSNodeChange change, UFIVSNode* Node);
	void OnSelectionChanged(UFIVSNode* InNode, bool bIsSelected);

	void DrawGrid(uint32 LayerId, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, const FWidgetStyle& InWidgetStyle) const;
	void DrawConnections(uint32 LayerId, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, const FWidgetStyle& InWidgetStyle) const;
	void DrawNewConnection(uint32 LayerId, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, const FWidgetStyle& InWidgetStyle) const;
	void DrawSelectionBox(uint32 LayerId, const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, const FWidgetStyle& InWidgetStyle) const;
};

class FFIVSEdGraphPanDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFIVSEdGraphPanDragDrop, FDragDropOperation)

	FFIVSEdGraphPanDragDrop(TSharedRef<SFIVSEdGraphViewer> GraphViewer);

	virtual FCursorReply OnCursorQuery() override;
	virtual void OnDragged(const FDragDropEvent& DragDropEvent) override;

private:
	TSharedRef<SFIVSEdGraphViewer> GraphViewer;
	TOptional<FVector2D> prevScreenPos;
};

class FFIVSEdGraphSelectionDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFIVSEdSelectionBoxDragDrop, FDragDropOperation)
	
	FFIVSEdGraphSelectionDragDrop(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, FVector2D StartPos);

	virtual void OnDragged(const class FDragDropEvent& DragDropEvent) override;
	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	TSharedRef<SFIVSEdGraphViewer> GetGraphViewer() const { return GraphViewer; }

private:
	TSharedRef<SFIVSEdGraphViewer> GraphViewer;
};
