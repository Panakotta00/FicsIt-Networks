#pragma once

#include "SlateBasics.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSNode.h"

class SFIVSEdPinViewer : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFIVSEdPinViewer) {}
		SLATE_ARGUMENT(UFIVSPin*, Pin)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	UFIVSPin* Pin;
	TSharedPtr<SImage> PinIconWidget;
	
public:
	SFIVSEdPinViewer();
	
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

	FSlateColor GetPinColor() const;

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

class SFIVSEdNodeViewer : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFIVSEdNodeViewer) {}
		SLATE_ATTRIBUTE(UFIVSNode*, Node)
	SLATE_END_ARGS()

public:
	void Construct( const FArguments& InArgs );

private:
	UFIVSNode* Node = nullptr;

	TSharedPtr<SVerticalBox> InputPinBox;
	TSharedPtr<SVerticalBox> OutputPinBox;
	
	TArray<TSharedRef<SFIVSEdPinViewer>> PinWidgets;
	TMap<UFIVSPin*, TSharedRef<SFIVSEdPinViewer>> PinToWidget;

	FSlateColorBrush OutlineBrush = FSlateColorBrush(FLinearColor(1,1,1));
	FSlateColorBrush NodeBrush = FSlateColorBrush(FColor::FromHex("222222"));
	FSlateColorBrush HeaderBrush = FSlateColorBrush(FColor::FromHex("b04600"));

	UFIVSPin* PinUnderMouse = nullptr;

public:
	bool bSelected = false;
	
	SFIVSEdNodeViewer();
	
	// Begin SWidget
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

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
	
	UFIVSPin* GetPinUnderMouse();
	TSharedRef<SFIVSEdPinViewer> GetPinWidget(UFIVSPin* Pin);
};
