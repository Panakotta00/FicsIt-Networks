#pragma once

#include "CoreMinimal.h"
#include "FIVSEdActionSelection.h"
#include "FIVSEdNodeViewer.h"
#include "SlateBasics.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSGraph.h"

class SFIVSEdGraphViewer;

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

	void GetRect(FVector2D& OutStartPos, FVector2D& OutEndPos) const {
		OutStartPos = StartPos;
		OutEndPos = EndPos;
	}

private:
	bool bIsActive = false;
	FVector2D StartPos;
	FVector2D EndPos;
};

class SFIVSEdGraphViewer : public SPanel {
	SLATE_BEGIN_ARGS(SFIVSEdGraphViewer) :
		_Style(&FFIVSEdStyle::GetDefault()) {}
	SLATE_STYLE_ARGUMENT(FFIVSEdStyle, Style)
	SLATE_ARGUMENT(UFIVSGraph*, Graph)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs );

private:
	const FFIVSEdStyle* Style = nullptr;
	UFIVSGraph* Graph = nullptr;
	TSlotlessChildren<SFIVSEdNodeViewer> Children;
	TMap<UFIVSNode*, TSharedRef<SFIVSEdNodeViewer>> NodeToChild;

	bool bIsGraphDrag = false;
	float GraphDragDelta = 0;

	TArray<TSharedRef<SFIVSEdPinViewer>> DraggingPins;
	FVector2D DraggingPinsEndpoint;
	
	TSharedPtr<FFIVSEdConnectionDrawer> ConnectionDrawer;

	TSharedPtr<SFIVSEdActionSelection> ActiveActionSelection;

	FSlateColorBrush BackgroundBrush = FSlateColorBrush(FColor::FromHex("040404"));
	FLinearColor GridColor = FColor::FromHex("0A0A0A");
	FSlateColorBrush SelectionBrush = FSlateColorBrush(FLinearColor(1,1,1,0.1));

	void OnSelectionChanged(UFIVSNode* InNode, bool bIsSelected);

public:
	FFIVSEdSelectionManager SelectionManager;
	FFIVSEdSelectionBoxHandler SelectionBoxHandler;
	
	FVector2D Offset = FVector2D(0,0);
	float Zoom = 1.0;
	
	SFIVSEdGraphViewer();
	~SFIVSEdGraphViewer();

	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
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
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

	void BeginDragPin(TSharedRef<SFIVSEdPinViewer> PinViewer) {
		DraggingPins.Add(PinViewer);
	}

	void EndDragPin(TSharedRef<SFIVSEdPinViewer> PinViewer) {
		DraggingPins.Remove(PinViewer);
	}

	FDelegateHandle OnNodeChangedHandle;
	void OnNodeChanged(int change, UFIVSNode* Node);

	/**
	 * Creates an action selection menu with the given context
	 */
	TSharedPtr<IMenu> CreateActionSelectionMenu(const FWidgetPath& Path, const FVector2D& Location, TFunction<void(const TSharedPtr<FFIVSEdActionSelectionAction>&)> OnExecute, const FFINScriptNodeCreationContext& Context);
	
	/**
	 * Sets the currently displayed Graph.
	 * Causes all child widgets to get regenerated.
	 *
	 * @param[in]	NewGraph	the new graph to display
	 */
	void SetGraph(UFIVSGraph* NewGraph);

	/**
	 * Adds the node at the given index in the graph
	 * as widget to children.
	 *
	 * @pragma[in]	Node
	 */
	void CreateNodeAsChild(UFIVSNode* Node);

	/**
	 * Converts the given local position to the position in the graph
	 */
	FVector2D LocalToGraph(const FVector2D Local);
};

class FFIVSEdSelectionBoxDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFIVSEdSelectionBoxDragDrop, FDragDropOperation)
	
	FFIVSEdSelectionBoxDragDrop(FFIVSEdSelectionBoxHandler& SelectionBoxHandler, FVector2D StartPos) : SelectionBoxHandler(SelectionBoxHandler) {
		SelectionBoxHandler.BeginDrag(StartPos);
	}

	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	FFIVSEdSelectionBoxHandler& GetSelectionBoxHandler() { return SelectionBoxHandler; }
	
private:
	FFIVSEdSelectionBoxHandler& SelectionBoxHandler;
};
