#pragma once

#include "CoreMinimal.h"
#include "FIVSEdEditor.h"
#include "FIVSEdGraphViewerStyle.h"
#include "FIVSScriptContext.h"
#include "SlateBasics.h"

class UFIVSNode;
class UFIVSPin;
class SFIVSEdGraphViewer;
class SFIVSEdNodeViewer;

class SFIVSEdPinViewer : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFIVSEdPinViewer) :
		_Style(&FFIVSEdNodeStyle::GetDefault()) {}
		SLATE_STYLE_ARGUMENT(FFIVSEdNodeStyle, Style)
		SLATE_ARGUMENT(UFIVSEdEditor*, Context)
		SLATE_ARGUMENT_DEFAULT(bool, ShowName) = true;
	SLATE_END_ARGS()
	
public:
	void Construct(const FArguments& InArgs, const TSharedRef<SFIVSEdNodeViewer>& NodeViewer, UFIVSPin* Pin);
	
private:
	UFIVSPin* Pin = nullptr;
	UFIVSEdEditor* Context;
	TWeakPtr<SFIVSEdNodeViewer> NodeViewer;
	TSharedPtr<SWidget> PinIconWidget;
	const FFIVSEdNodeStyle* Style = nullptr;
	
public:
	SFIVSEdPinViewer();
	
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	// End SWidget
	
	FSlateColor GetPinColor() const;

	TSharedPtr<SFIVSEdNodeViewer> GetNodeViewer() { return NodeViewer.Pin(); }
	
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
protected:
	TSharedRef<SBorder> Construct(const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, UFIVSNode* Node, const FFIVSEdNodeStyle* InStyle);
	
protected:
	UFIVSNode* Node = nullptr;
	TWeakPtr<SFIVSEdGraphViewer> GraphViewer;
	const FFIVSEdNodeStyle* Style = nullptr;
	FDelegateHandle OnPinChangeHandle;
	
protected:
	TArray<TSharedRef<SFIVSEdPinViewer>> PinWidgets;
	TMap<UFIVSPin*, TSharedRef<SFIVSEdPinViewer>> PinToWidget;
	
public:
	bool bSelected = false;

	virtual ~SFIVSEdNodeViewer() override;
	
	// Begin SWidget
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	// End SWidget

	/**
	 * This function will be called if something requests a reconstruction of the pins (like pin changes).
	 * Implementations may use it for initial construction too.
	 */
	virtual void ReconstructPins() = 0;

	TSharedPtr<SFIVSEdGraphViewer> GetGraphViewer() { return GraphViewer.Pin(); }
	
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
	const TArray<TSharedRef<SFIVSEdPinViewer>>& GetPinWidgets() const;
	
	TSharedRef<SFIVSEdPinViewer> GetPinWidget(UFIVSPin* Pin) const;
};

class SFIVSEdRerouteNodeViewer : public SFIVSEdNodeViewer {
	SLATE_BEGIN_ARGS(SFIVSEdRerouteNodeViewer) :
		_Style(&FFIVSEdNodeStyle::GetDefault()) {}
		SLATE_STYLE_ARGUMENT(FFIVSEdNodeStyle, Style)
		SLATE_ARGUMENT(UFIVSEdEditor*, Context)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs, const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, UFIVSNode* Node);

	UFIVSEdEditor* Context = nullptr;
public:
	virtual void ReconstructPins() override {}
};

class SFIVSEdFunctionNodeViewer : public SFIVSEdNodeViewer {
	SLATE_BEGIN_ARGS(SFIVSEdFunctionNodeViewer) :
		_Style(&FFIVSEdNodeStyle::GetDefault()) {}
		SLATE_STYLE_ARGUMENT(FFIVSEdNodeStyle, Style)
		SLATE_ARGUMENT(UFIVSEdEditor*, Context)
		SLATE_DEFAULT_SLOT(FArguments, Footer)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, UFIVSNode* Node);

protected:
	UFIVSEdEditor* Context = nullptr;
	TSharedPtr<SVerticalBox> InputPinBox;
	TSharedPtr<SVerticalBox> OutputPinBox;

	virtual bool ShowName() const;

public:
	virtual void ReconstructPins() override;
};

class SFIVSEdOperatorNodeViewer : public SFIVSEdFunctionNodeViewer {
	SLATE_BEGIN_ARGS(SFIVSEdOperatorNodeViewer) :
		_Style(&FFIVSEdNodeStyle::GetDefault()) {}
		SLATE_STYLE_ARGUMENT(FFIVSEdNodeStyle, Style)
		SLATE_ARGUMENT(UFIVSEdEditor*, Context)
		SLATE_ARGUMENT(FString, Symbol)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<SFIVSEdGraphViewer>& GraphViewer, UFIVSNode* Node);

protected:
	UFIVSEdEditor* Context = nullptr;

	virtual bool ShowName() const override;
};

class FFIVSEdNodeDragDrop : public FDragDropOperation {
public:
	DRAG_DROP_OPERATOR_TYPE(FFIVSEdNodeDragDrop, FDragDropOperation)
	
	FFIVSEdNodeDragDrop(TSharedRef<SFIVSEdNodeViewer> InNodeViewer);

	virtual void OnDragged(const FDragDropEvent& DragDropEvent) override;

	TSharedRef<SFIVSEdNodeViewer> GetNodeViewer() { return NodeViewer; }
	
private:
	TSharedRef<SFIVSEdNodeViewer> NodeViewer;
	TOptional<FVector2D> PreviousGraphPos;
	FVector2D Delta;
	TArray<UFIVSNode*> Nodes;
	TArray<FVector2D> NodeStartPos;
};
