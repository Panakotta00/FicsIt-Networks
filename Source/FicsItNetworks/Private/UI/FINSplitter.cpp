#include "UI/FINSplitter.h"

#include "ArrangedChildren.h"

void SFINSplitter::Construct(FArguments InArgs) {
	SSplitter::FArguments Args;
	Args._Clipping = InArgs._Clipping;
	Args._Cursor = InArgs._Cursor;
	Args._FlowDirectionPreference = InArgs._FlowDirectionPreference;
	Args._ForceVolatile = InArgs._ForceVolatile;
	Args._HitDetectionSplitterHandleSize = InArgs._HitDetectionSplitterHandleSize;
	Args._IsEnabled = InArgs._IsEnabled;
	Args._MinimumSlotHeight = InArgs._MinimumSlotHeight;
	Args._OnGetMaxSlotSize = InArgs._OnGetMaxSlotSize;
	Args._OnSplitterFinishedResizing = InArgs._OnSplitterFinishedResizing;
	Args._PhysicalSplitterHandleSize = InArgs._PhysicalSplitterHandleSize;
	Args._RenderOpacity = InArgs._RenderOpacity;
	Args._RenderTransform = InArgs._RenderTransform;
	Args._RenderTransformPivot = InArgs._RenderTransformPivot;
	Args._ResizeMode = InArgs._ResizeMode;
	Args._Style = InArgs._Style;
	Args._Tag = InArgs._Tag;
	Args._ToolTip = InArgs._ToolTip;
	Args._ToolTipText = InArgs._ToolTipText;
	Args._Visibility = InArgs._Visibility;
	//Args._Slots = MoveTemp(InArgs._Slots);
	Args.MetaData = InArgs.MetaData;
	SSplitter::Construct(Args);

	Style = InArgs._Style;
}

int32 SFINSplitter::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	int MaxLayerId = SSplitter::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	MaxLayerId++;
	
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(AllottedGeometry, ArrangedChildren);
	
	for( int32 ChildIndex = 0; ChildIndex < ArrangedChildren.Num(); ++ChildIndex ) {
		const FGeometry& GeometryAfterSplitter = ArrangedChildren[FMath::Clamp(ChildIndex + 1, 0, ArrangedChildren.Num()-1)].Geometry;

		FVector2D HandleSize;		
		FVector2D HandlePosition;
		HandleSize.Set( PhysicalSplitterHandleSize, PhysicalSplitterHandleSize );
		HandlePosition.Set( -HandleSize.X, (GeometryAfterSplitter.Size.Y / 2.0) - HandleSize.Y );

		FSlateDrawElement::MakeBox(
            OutDrawElements,
            MaxLayerId,
            GeometryAfterSplitter.ToPaintGeometry( HandleSize, FSlateLayoutTransform(1.0f, HandlePosition)),
            &Style->HandleIconBrush,
            ShouldBeEnabled(bParentEnabled) ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect,
            InWidgetStyle.GetColorAndOpacityTint() * Style->HandleHighlightBrush.TintColor.GetSpecifiedColor()
        );
	}

	return MaxLayerId;
}
