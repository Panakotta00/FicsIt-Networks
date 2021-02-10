#pragma once

#include "CoreMinimal.h"
#include "FIVSEdActionSelection.h"
#include "FIVSEdNodeViewer.h"
#include "SlateBasics.h"
#include "FicsItVisualScript/Script/FIVSGraph.h"

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

class SFIVSEdGraphViewer : public SPanel {
	SLATE_BEGIN_ARGS(SFIVSEdGraphViewer) {}
		SLATE_ARGUMENT(UFIVSGraph*, Graph)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs );

private:
	UFIVSGraph* Graph = nullptr;
	TSlotlessChildren<SFIVSEdNodeViewer> Children;
	TMap<UFIVSNode*, TSharedRef<SFIVSEdNodeViewer>> NodeToChild;

	UFIVSNode* NodeUnderMouse = nullptr;
	UFIVSPin* PinUnderMouse = nullptr;

	bool bIsGraphDrag = false;
	float GraphDragDelta;

	bool bIsSelectionDrag = false;
	FVector2D SelectionDragStart;
	FVector2D SelectionDragEnd;
	
	TArray<UFIVSNode*> SelectedNodes;
	
	bool bIsNodeDrag = false;
	TArray<FVector2D> NodeDragPosStart;
	FVector2D NodeDragStart;

	bool bIsPinDrag = false;
	UFIVSPin* PinDragStart;
	FVector2D PinDragEnd;

	TSharedPtr<FFIVSEdConnectionDrawer> ConnectionDrawer;

	TSharedPtr<SFIVSEdActionSelection> ActiveActionSelection;

	FSlateColorBrush BackgroundBrush = FSlateColorBrush(FColor::FromHex("040404"));
	FLinearColor GridColor = FColor::FromHex("0A0A0A");
	FSlateColorBrush SelectionBrush = FSlateColorBrush(FLinearColor(1,1,1,0.1));

public:
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
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

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
	 * Selects the given node
	 */
	void Select(UFIVSNode* Node);

	/**
	 * Deselects the given node
	 */
	void Deselect(UFIVSNode* Node);

	/**
	 * Deselects all nodes.
	 */
	void DeselectAll();

	/**
	 * Converts the given local position to the position in the graph
	 */
	FVector2D LocalToGraph(const FVector2D Local);
};
