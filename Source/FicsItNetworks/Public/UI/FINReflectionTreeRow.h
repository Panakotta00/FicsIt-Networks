#pragma once

#include "FINReflectionStyles.h"

template<typename ItemType>
class SFINReflectionTreeRow : public STableRow<ItemType> {
	SLATE_BEGIN_ARGS(SFINReflectionTreeRow<ItemType>) : _Content() {}
		SLATE_DEFAULT_SLOT(typename SFINReflectionTreeRow<ItemType>::FArguments, Content)
		SLATE_STYLE_ARGUMENT( FTableRowStyle, Style )
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView) {
		typename STableRow<ItemType>::FArguments SuperArgs;
		SuperArgs._Content = InArgs._Content;
		SuperArgs._ExpanderStyleSet = &FFINReflectionStyles::Get();
		SuperArgs._Style = InArgs._Style;
		STableRow<ItemType>::Construct(SuperArgs, InOwnerTableView);
	}
	
	virtual void ConstructChildren(ETableViewMode::Type InOwnerTableMode, const TAttribute<FMargin>& InPadding, const TSharedRef<SWidget>& InContent) {
		this->Content = InContent;
		this->InnerContentSlot = nullptr;

		SHorizontalBox::FSlot* InnerContentSlotNativePtr = nullptr;

		this->ChildSlot[
            SNew(SHorizontalBox)
            +SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Right)
            .VAlign(VAlign_Fill)[
                SNew(SExpanderArrow, SharedThis(this) )
                .StyleSet(this->ExpanderStyleSet)
            ]
            + SHorizontalBox::Slot()
            .FillWidth(1)
            .Expose(InnerContentSlotNativePtr)
            .Padding(InPadding)[
                InContent
            ]
        ];

		this->InnerContentSlot = InnerContentSlotNativePtr;
	}
};