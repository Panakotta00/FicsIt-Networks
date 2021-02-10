#pragma once

#include "SlateBasics.h"
#include "SPanel.h"
#include "FINScriptNode.h"

class SFINScriptPinViewer : public SPanel {
	SLATE_BEGIN_ARGS(SFINScriptPinViewer) {}
		SLATE_ARGUMENT(UFINScriptPin*, Pin)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	UFINScriptPin* Pin;
	TSlotlessChildren<SBorder> Children;
	TSharedPtr<SImage> PinIconWidget;
	
public:
	SFINScriptPinViewer();
	
	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

	FSlateColor GetPinColor() const;

	/**
	 * Sets the representating pin
	 *
	 * @param[in]	newPin	the pin we should display
	 */
	void SetPin(UFINScriptPin* newPin);

	/**
	 * Returns the representating pin.
	 */
	UFINScriptPin* GetPin() const;

	/**
	 * Returns the connection point in screen space.
	 */
	FVector2D GetConnectionPoint() const;
};

class SFINScriptNodeViewer : public SPanel {
	SLATE_BEGIN_ARGS(SFINScriptNodeViewer) {}
		SLATE_ATTRIBUTE(UFINScriptNode*, Node)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs );

private:
	UFINScriptNode* Node = nullptr;
	TSlotlessChildren<SWidget> Children;

	TSharedPtr<SVerticalBox> InputPinBox;
	TSharedPtr<SVerticalBox> OutputPinBox;
	
	TArray<TSharedRef<SFINScriptPinViewer>> PinWidgets;
	TMap<UFINScriptPin*, TSharedRef<SFINScriptPinViewer>> PinToWidget;

	FSlateColorBrush OutlineBrush = FSlateColorBrush(FLinearColor(1,1,1));
	FSlateColorBrush NodeBrush = FSlateColorBrush(FColor::FromHex("222222"));
	FSlateColorBrush HeaderBrush = FSlateColorBrush(FColor::FromHex("b04600"));

	UFINScriptPin* PinUnderMouse = nullptr;

public:
	bool bSelected = false;
	
	SFINScriptNodeViewer();
	
	// Begin SWidget
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

	/**
	 * Sets the node we representate.
	 *
	 * @param[in]	newNode		the new node
	 */
	void SetNode(UFINScriptNode* newNode);

	/**
	 * Returns the node we representate.
	 */
	UFINScriptNode* GetNode() const;
	
	/**
	 * Returns the position were this node should be located in graph.
	 */
	FVector2D GetPosition() const;
	
	/**
	 * Returns the pin widget cache.
	 */
	const TArray<TSharedRef<SFINScriptPinViewer>>& GetPinWidgets();
	
	UFINScriptPin* GetPinUnderMouse();
	TSharedRef<SFINScriptPinViewer> GetPinWidget(UFINScriptPin* Pin);
};
