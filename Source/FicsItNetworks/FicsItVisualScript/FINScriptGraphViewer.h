#pragma once

#include "CoreMinimal.h"

#include "FINScriptGraph.h"
#include "FINScriptNodeViewer.h"
#include "SlateBasics.h"

class SFINScriptGraphViewer : public SPanel {
	SLATE_BEGIN_ARGS(SFINScriptGraphViewer) {}
		SLATE_ATTRIBUTE(UFINScriptGraph*, Graph)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs );

private:
	UFINScriptGraph* Graph = nullptr;
	TSlotlessChildren<SFINScriptNodeViewer> Children;
	TMap<UFINScriptNode*, TSharedRef<SFINScriptNodeViewer>> NodeToChild;

	FVector2D Offset;

	UFINScriptNode* NodeUnderMouse = nullptr;
	TSharedPtr<FFINScriptPin> PinUnderMouse = nullptr;

	bool bIsGraphDrag = false;
	
	TArray<UFINScriptNode*> SelectedNodes;
	
	bool bIsNodeDrag = false;
	TArray<FVector2D> NodeDragPosStart;
	FVector2D NodeDragStart;

	bool bIsPinDrag = false;
	TSharedPtr<FFINScriptPin> PinDragStart;
	FVector2D PinDragEnd;

	FSlateColorBrush BackgroundBrush = FSlateColorBrush(FColor::FromHex("040404"));
	FLinearColor GridColor = FColor::FromHex("0A0A0A");

public:
	SFINScriptGraphViewer();
	~SFINScriptGraphViewer();

	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// End SWidget

	FDelegateHandle OnNodeChangedHandle;
	void OnNodeChanged(int change, UFINScriptNode* Node);
	
	/**
	 * Sets the currently displayed Graph.
	 * Causes all child widgets to get regenerated.
	 *
	 * @param[in]	NewGraph	the new graph to display
	 */
	void SetGraph(UFINScriptGraph* NewGraph);

	/**
	 * Adds the node at the given index in the graph
	 * as widget to children.
	 *
	 * @pragma[in]	Node
	 */
	void CreateNodeAsChild(UFINScriptNode* Node);

	/**
	 * Selects the given node
	 */
	void Select(UFINScriptNode* Node);

	/**
	 * Deselects the given node
	 */
	void Deselect(UFINScriptNode* Node);

	/**
	 * Deselects all nodes.
	 */
	void DeselectAll();
};
