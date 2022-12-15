#pragma once

#include "FINReflectionUIStyle.h"
#include "Widgets/Layout/SSplitter.h"

class SFINSplitter : public SSplitter {
	SLATE_BEGIN_ARGS(SFINSplitter)
        : _Style(nullptr)
		, _Orientation( Orient_Horizontal )
        , _ResizeMode( ESplitterResizeMode::FixedPosition )
        , _PhysicalSplitterHandleSize( 5.0f )
        , _HitDetectionSplitterHandleSize( 5.0f )
        , _MinimumSlotHeight( 20.0f )
        , _OnSplitterFinishedResizing()
	{
	}

	SLATE_SUPPORTS_SLOT(FSlot)

    SLATE_STYLE_ARGUMENT( FFINSplitterStyle, Style )
    SLATE_ARGUMENT( EOrientation, Orientation )
    SLATE_ARGUMENT( ESplitterResizeMode::Type, ResizeMode )
    SLATE_ARGUMENT( float, PhysicalSplitterHandleSize )
    SLATE_ARGUMENT( float, HitDetectionSplitterHandleSize )
    SLATE_ARGUMENT( float, MinimumSlotHeight )
    SLATE_EVENT( FSimpleDelegate, OnSplitterFinishedResizing )
    SLATE_EVENT(FOnGetMaxSlotSize, OnGetMaxSlotSize)
SLATE_END_ARGS()
private:
	const FFINSplitterStyle* Style = nullptr;

public:
	void Construct(const FArguments& InArgs);
	
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
};
