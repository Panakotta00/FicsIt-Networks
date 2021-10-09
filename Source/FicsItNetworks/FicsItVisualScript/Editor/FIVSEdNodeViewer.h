#pragma once

#include "FIVSEdStyle.h"
#include "SlateBasics.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSNode.h"

class SFIVSEdGraphViewer;
class SFIVSEdNodeViewer;

class SFIVSEdPinViewer : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFIVSEdPinViewer) :
		_Style(&FFIVSEdStyle::GetDefault()) {}
	SLATE_STYLE_ARGUMENT(FFIVSEdStyle, Style)
	SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& InArgs, SFIVSEdNodeViewer* NodeViewer, UFIVSPin* Pin);
	
private:
	UFIVSPin* Pin;
	SFIVSEdNodeViewer* NodeViewer;
	TSharedPtr<SWidget> PinIconWidget;
	const FFIVSEdStyle* Style = nullptr;
	
public:
	SFIVSEdPinViewer();
	
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// End SWidget
	
	FSlateColor GetPinColor() const;

	SFIVSEdNodeViewer* GetNodeViewer() { return NodeViewer; }
	
	/**
	 * Sets the representating pin
	 *
	 * @param[in]	newPin	the pin we should display
	 */
	void SetPin(UFIVSPin* newPin);
	
	/**
	 * Returns the representating pin.
	 */
	UFIVSPin* GetPin() const;
	
	/**
	 * Returns the connection point in screen space.
	 */
	FVector2D GetConnectionPoint() const;
};

class FFIVSEdPinConnectDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFIVSEdPinConnectDragDrop, FDragDropOperation)
	
	FFIVSEdPinConnectDragDrop(TSharedRef<SFIVSEdPinViewer> Pin);

	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	TSharedRef<SFIVSEdPinViewer> GetPinViewer() { return Pin; }

private:
	TSharedRef<SFIVSEdPinViewer> Pin;
};

class SFIVSEdNodeViewer : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFIVSEdNodeViewer) :
		_Style(&FFIVSEdStyle::GetDefault()) {}
	SLATE_STYLE_ARGUMENT(FFIVSEdStyle, Style)
	SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& InArgs, SFIVSEdGraphViewer* GraphViewer, UFIVSNode* Node);
	
private:
	const FFIVSEdStyle* Style = nullptr;
	UFIVSNode* Node = nullptr;
	SFIVSEdGraphViewer* GraphViewer;
	
	TSharedPtr<SVerticalBox> InputPinBox;
	TSharedPtr<SVerticalBox> OutputPinBox;
	
	TArray<TSharedRef<SFIVSEdPinViewer>> PinWidgets;
	TMap<UFIVSPin*, TSharedRef<SFIVSEdPinViewer>> PinToWidget;
	
	FSlateColorBrush OutlineBrush = FSlateColorBrush(FLinearColor(1,1,1));
	FSlateColorBrush NodeBrush = FSlateColorBrush(FColor::FromHex("222222"));
	FSlateColorBrush HeaderBrush = FSlateColorBrush(FColor::FromHex("b04600"));
	
public:
	bool bSelected = false;
	
	SFIVSEdNodeViewer();
	
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

	SFIVSEdGraphViewer* GetGraphViewer() { return GraphViewer; }
	
	/**
	 * Sets the node we representate.
	 *
	 * @param[in]	newNode		the new node
	 */
	void SetNode(UFIVSNode* newNode);
	
	/**
	 * Returns the node we representate.
	 */
	UFIVSNode* GetNode() const;
	
	/**
	 * Returns the position were this node should be located in graph.
	 */
	FVector2D GetPosition() const;
	
	/**
	 * Returns the pin widget cache.
	 */
	const TArray<TSharedRef<SFIVSEdPinViewer>>& GetPinWidgets();
	
	TSharedRef<SFIVSEdPinViewer> GetPinWidget(UFIVSPin* Pin);
};

class FFIVSEdNodeDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFIVSEdNodeDragDrop, FDragDropOperation)
	
	FFIVSEdNodeDragDrop(TSharedRef<SFIVSEdNodeViewer> InNodeViewer);

	virtual void OnDragged(const FDragDropEvent& DragDropEvent) override;

	TSharedRef<SFIVSEdNodeViewer> GetNodeViewer() { return NodeViewer; }
	
private:
	TSharedRef<SFIVSEdNodeViewer> NodeViewer;
};