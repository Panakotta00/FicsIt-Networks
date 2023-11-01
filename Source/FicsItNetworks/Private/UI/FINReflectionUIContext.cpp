#include "UI/FINReflectionUIContext.h"

#include "Network/FINNetworkValues.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINReflection.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"
#include "UI/FINReflectionClassHirachyViewer.h"
#include "UI/FINReflectionEntryListViewer.h"
#include "UI/FINSplitter.h"
#include "Widgets/Layout/SScaleBox.h"

FString GetText(UFINProperty* Prop) {
	if (!Prop) return FString();
	switch (Prop->GetType()) {
	case FIN_ANY:
		return "Any";
	case FIN_NIL:
		return "Nil";
	case FIN_BOOL:
		return "Bool";
	case FIN_INT:
		return "Int";
	case FIN_FLOAT:
		return "Float";
	case FIN_STR:
		return "Str";
	case FIN_OBJ:
		if (Cast<UFINObjectProperty>(Prop)->GetSubclass()) {
			return FString("Obj(") + FFINReflection::Get()->FindClass(Cast<UFINObjectProperty>(Prop)->GetSubclass())->GetDisplayName().ToString() + ")";
		}
		return "Obj";
	case FIN_CLASS:
		if (Cast<UFINClassProperty>(Prop)->GetSubclass()) {
			return FString("Class(") + FFINReflection::Get()->FindClass(Cast<UFINClassProperty>(Prop)->GetSubclass())->GetDisplayName().ToString() + ")";
		}
		return "Class";
	case FIN_STRUCT:
		if (Cast<UFINStructProperty>(Prop)->Struct) {
			return FString("Struct(") +  FFINReflection::Get()->FindStruct(Cast<UFINStructProperty>(Prop)->Struct)->GetDisplayName().ToString() + ")";
		}
		return "Struct";
	case FIN_TRACE:
		if (Cast<UFINTraceProperty>(Prop)->GetSubclass()) {
			return FString("Trace(") + FFINReflection::Get()->FindClass(Cast<UFINTraceProperty>(Prop)->GetSubclass())->GetDisplayName().ToString() + ")";
		}
		return "Trace";
	case FIN_ARRAY:
		return FString("Array(") + GetText(Cast<UFINArrayProperty>(Prop)->GetInnerType()) + ")";
	default:
		return FString();
	}
}

TSharedRef<SWidget> GenerateDataTypeIcon(UFINProperty* Prop, FFINReflectionUIContext* Context) {
	const FFINReflectionUIStyleStruct* Style = Context->Style.Get();
	check(Style != nullptr);
	TSharedPtr<SWidget> Widget;
	switch (Prop->GetType()) {
	case FIN_OBJ: {
		UFINClass* Class = FFINReflection::Get()->FindClass(Cast<UFINObjectProperty>(Prop)->GetSubclass());
		if (Class) SAssignNew(Widget, SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString("Obj("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Cursor(EMouseCursor::Hand)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .ColorAndOpacity(Context->Style.Get()->ClickableDataTypeColor)
            .Text(Class->GetDisplayName())
            .OnDoubleClicked_Lambda([Context, Class](const FGeometry&, const FPointerEvent&) {
                Context->NavigateTo(Context->Structs.Find(Class)->Get());
                return FReply::Handled();
            })]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString(")"))
        ]];
		break;
	} case FIN_CLASS: {
		UFINClass* Class = FFINReflection::Get()->FindClass(Cast<UFINClassProperty>(Prop)->GetSubclass());
		if (Class) SAssignNew(Widget, SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString("Class("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Cursor(EMouseCursor::Hand)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .ColorAndOpacity(Context->Style.Get()->ClickableDataTypeColor)
            .Text(Class->GetDisplayName())
            .OnDoubleClicked_Lambda([Context, Class](const FGeometry&, const FPointerEvent&) {
                Context->NavigateTo(Context->Structs.Find(Class)->Get());
                return FReply::Handled();
            })]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString(")"))
        ]];
		break;
	} case FIN_TRACE: {
		UFINClass* Class = FFINReflection::Get()->FindClass(Cast<UFINTraceProperty>(Prop)->GetSubclass());
		if (Class) SAssignNew(Widget, SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString("Trace("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Cursor(EMouseCursor::Hand)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .ColorAndOpacity(Context->Style.Get()->ClickableDataTypeColor)
            .Text(Class->GetDisplayName())
            .OnDoubleClicked_Lambda([Context, Class](const FGeometry&, const FPointerEvent&) {
                Context->NavigateTo(Context->Structs.Find(Class)->Get());
                return FReply::Handled();
            })]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString(")"))
        ]];
		break;
	} case FIN_STRUCT: {
		UFINStruct* Struct = FFINReflection::Get()->FindStruct(Cast<UFINStructProperty>(Prop)->GetInner());
		if (Struct) SAssignNew(Widget, SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString("Struct("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Cursor(EMouseCursor::Hand)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .ColorAndOpacity(Context->Style.Get()->ClickableDataTypeColor)
            .Text(Struct->GetDisplayName())
            .OnDoubleClicked_Lambda([Context, Struct](const FGeometry&, const FPointerEvent&) {
                Context->NavigateTo(Context->Structs.Find(Struct)->Get());
                return FReply::Handled();
            })]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString(")"))
        ]];
		break;
	} case FIN_ARRAY: {
		SAssignNew(Widget, SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString("Array("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
			GenerateDataTypeIcon(Cast<UFINArrayProperty>(Prop)->GetInnerType(), Context)
		]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->DataTypeTextStyle)
            .Text(FText::FromString(")"))
        ]];
		break;
	} default: ; }

	FString Text = GetText(Prop);
	if (!Widget.IsValid()) SAssignNew(Widget, STextBlock).Text(FText::FromString(Text)).TextStyle(&Context->Style.Get()->DataTypeTextStyle);
	
	if (Prop->GetPropertyFlags() & FIN_Prop_OutParam || Prop->GetPropertyFlags() & FIN_Prop_RetVal) {
		return SNew(SHorizontalBox)
		+SHorizontalBox::Slot().VAlign(VAlign_Center).AutoWidth().Padding(0,0,5,0)[
			SNew(STextBlock)
			.TextStyle(&Context->Style.Get()->FlagsTextStyle)
			.ColorAndOpacity(Context->Style.Get()->OutFlagColor)
			.Text(FText::FromString("out"))
			.ToolTipText(FText::FromString("Output Parameter - This parameter is returned by the function."))
		]
		+SHorizontalBox::Slot().VAlign(VAlign_Center).AutoWidth()[
			Widget.ToSharedRef()
		];
	}
	return Widget.ToSharedRef();
}

TSharedRef<SWidget> GeneratePropTypeIcon(UFINProperty* Prop, FFINReflectionUIContext* Context) {
	const FFINReflectionUIStyleStruct* Style = Context->Style.Get();
	check(Style != nullptr);
	TSharedRef<STextBlock> Text = SNew(STextBlock)
	.TextStyle(&Context->Style.Get()->FlagsTextStyle)
	.Text(FText::FromString("mp"))
	.ToolTipText(FText::FromString("Member Property - A property that can only be changed on an instance."))
	.ColorAndOpacity(Context->Style.Get()->PropertyColor);
	if (Prop->GetPropertyFlags() & FIN_Prop_ClassProp) {
		Text->SetText(FText::FromString("cp"));
		Text->SetToolTipText(FText::FromString("Class Property - A property that can only be changed on a class."));
	}
	return SNew(SBox).WidthOverride(30).Content()[Text];
}

TSharedRef<SWidget> GenerateFuncTypeIcon(UFINFunction* Func, FFINReflectionUIContext* Context) {
	const FFINReflectionUIStyleStruct* Style = Context->Style.Get();
	check(Style != nullptr);
	TSharedRef<STextBlock> Text = SNew(STextBlock)
	.TextStyle(&Context->Style.Get()->FlagsTextStyle)
	.Text(FText::FromString("mf"))
	.ToolTipText(FText::FromString("Member Function - A function that can only be called on an instance."))
	.ColorAndOpacity(Context->Style.Get()->FunctionColor);
	if (Func->GetFunctionFlags() & FIN_Func_ClassFunc) {
		Text->SetText(FText::FromString("cf"));
		Text->SetToolTipText(FText::FromString("Class Function - A function that can only be called on the class."));
	}
	if (Func->GetFunctionFlags() & FIN_Func_StaticFunc) {
		Text->SetText(FText::FromString("sf"));
		Text->SetToolTipText(FText::FromString("Static Function - A function that can be called anytime."));
	}
	return SNew(SBox).WidthOverride(30).Content()[Text];
}

TSharedRef<SWidget> GenerateSignalTypeIcon(UFINSignal* Signal, FFINReflectionUIContext* Context) {
	const FFINReflectionUIStyleStruct* Style = Context->Style.Get();
	check(Style != nullptr);
	TSharedRef<STextBlock> Text = SNew(STextBlock)
	.TextStyle(&Context->Style.Get()->FlagsTextStyle)
	.Text(FText::FromString("ms"))
	.ToolTipText(FText::FromString("Member Signal - A signal that this object can emit."))
	.ColorAndOpacity(Context->Style.Get()->SignalColor);
	return SNew(SBox).WidthOverride(30).Content()[Text];
}

TSharedRef<SWidget> GenerateRuntimeFlag(int Runtime, FFINReflectionUIContext* Context) {
	const FFINReflectionUIStyleStruct* Style = Context->Style.Get();
	check(Style != nullptr);
	FString FlagText = "";
	FText ToolTipText;
	switch (Runtime) {
	case 0:
		FlagText = "rs";
		ToolTipText = FText::FromString("Runtime Synchronous - Can be called/changed in Game Tick.");
		break;
	case 1:
		FlagText = "rp";
		ToolTipText = FText::FromString("Runtime Parallel - Can be called/changed in Satisfactory Factory Tick.");
		break;
	case 2:
		FlagText = "ra";
		ToolTipText = FText::FromString("Runtime Asynchronous - Can be changed anytime.");
		break;
	default: break;
	}
	TSharedRef<STextBlock> Text = SNew(STextBlock)
    .TextStyle(&Context->Style.Get()->FlagsTextStyle)
    .Text(FText::FromString(FlagText))
	.ToolTipText(ToolTipText)
    .ColorAndOpacity(Context->Style.Get()->RuntimeFlagColor.GetSpecifiedColor() * (1+(Runtime * 0.5)));
	return Text;
}

TSharedRef<SWidget> GenerateFuncFlags(UFINFunction* Func, FFINReflectionUIContext* Context) {
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);
	if (Func->GetFunctionFlags() & FIN_Func_RT_Sync) {
		Box->AddSlot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		.Padding(5, 0, 0, 0)[
			GenerateRuntimeFlag(0, Context)
		];
	}
	if (Func->GetFunctionFlags() & FIN_Func_RT_Parallel) {
		Box->AddSlot()
        .VAlign(VAlign_Center)
        .AutoWidth()
		.Padding(5, 0, 0, 0)[
            GenerateRuntimeFlag(1, Context)
        ];
	}
	if (Func->GetFunctionFlags() & FIN_Func_RT_Async) {
		Box->AddSlot()
        .VAlign(VAlign_Center)
        .AutoWidth()
		.Padding(5, 0, 0, 0)[
            GenerateRuntimeFlag(2, Context)
        ];
	}
	if (Func->GetFunctionFlags() & FIN_Func_VarArgs) {
		Box->AddSlot()
        .VAlign(VAlign_Center)
        .AutoWidth()
		.Padding(5, 0, 0, 0)[
			SNew(STextBlock)
			.TextStyle(&Context->Style.Get()->FlagsTextStyle)
			.Text(FText::FromString("va"))
			.ToolTipText(FText::FromString("Variable Arguments - Can have any additional arguments as described."))
			.ColorAndOpacity(Context->Style.Get()->VarArgsFlagColor.GetSpecifiedColor())
        ];
	}
	if (Func->GetFunctionFlags() & FIN_Func_VarRets) {
		Box->AddSlot()
        .VAlign(VAlign_Center)
        .AutoWidth()
		.Padding(5, 0, 0, 0)[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->FlagsTextStyle)
            .Text(FText::FromString("vr"))
            .ToolTipText(FText::FromString("Variable Returns - Can have any additional return values as described."))
            .ColorAndOpacity(Context->Style.Get()->VarArgsFlagColor.GetSpecifiedColor())
        ];
	}
	return Box;
}


TSharedRef<SWidget> GeneratePropFlags(UFINProperty* Prop, FFINReflectionUIContext* Context) {
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);
	if (Prop->GetPropertyFlags() & FIN_Func_RT_Sync) {
		Box->AddSlot()
        .VAlign(VAlign_Center)
        .AutoWidth()
		.Padding(5, 0, 0, 0)[
            GenerateRuntimeFlag(0, Context)
        ];
	}
	if (Prop->GetPropertyFlags() & FIN_Prop_RT_Parallel) {
		Box->AddSlot()
        .VAlign(VAlign_Center)
        .AutoWidth()
		.Padding(5, 0, 0, 0)[
            GenerateRuntimeFlag(1, Context)
        ];
	}
	if (Prop->GetPropertyFlags() & FIN_Prop_RT_Async) {
		Box->AddSlot()
        .VAlign(VAlign_Center)
        .AutoWidth()
		.Padding(5, 0, 0, 0)[
            GenerateRuntimeFlag(2, Context)
        ];
	}
	if (Prop->GetPropertyFlags() & FIN_Prop_ReadOnly) {
		Box->AddSlot()
        .VAlign(VAlign_Center)
        .AutoWidth()
		.Padding(5, 0, 0, 0)[
            SNew(STextBlock)
            .TextStyle(&Context->Style.Get()->FlagsTextStyle)
            .Text(FText::FromString("ro"))
            .ToolTipText(FText::FromString("Read Only - The value of this property can not be changed by code."))
            .ColorAndOpacity(Context->Style.Get()->ReadOnlyFlagColor.GetSpecifiedColor())
        ];
	}
	return Box;
}

FFINReflectionUIFilter::FFINReflectionUIFilter(FString Filter) {
	FString Token;
	while (Filter.Split(" ", &Filter, &Token)) {
		if (Token.Len() > 0) Tokens.Add(Token);
	}
	if (Filter.Len() > 0) Tokens.Add(Filter);
}

bool FFINReflectionUIFilter::PassesFilter(const FString& String) const {
	for (const FString& Token : Tokens) {
		if (!String.Contains(Token)) return false;
	}
	return true;
}

void FFINReflectionUIEntry::UpdateChildren(bool bForce) {
	if (bUpdateChildren || bForce) {
		bUpdateChildren = false;
		LoadChildren();
	}
}

TSharedRef<SWidget> FFINReflectionUIEntry::GetLink() {
	return SNew(SButton)
	.ForegroundColor(FColor::Transparent)
	.ButtonColorAndOpacity(FColor::Transparent)
	.Content()[
		GetShortPreview()
	]
	.OnClicked_Lambda([this]() {
		this->Context->NavigateTo(this);
		return FReply::Handled();
	})
	.Cursor(EMouseCursor::Hand);
}

void FFINReflectionUIStruct::LoadChildren() {
	Attributes.Empty();
	Functions.Empty();
	for (UFINProperty* Prop : Struct->GetProperties(Context->GetShowRecursive())) {
		Attributes.Add(MakeShared<FFINReflectionUIProperty>(SharedThis(this), Prop, Context));
	}
	for (UFINFunction* Func : Struct->GetFunctions(Context->GetShowRecursive())) {
		Functions.Add(MakeShared<FFINReflectionUIFunction>(SharedThis(this), Func, Context));
	}
}

FFINReflectionUIStruct::FFINReflectionUIStruct(const TWeakPtr<FFINReflectionUIEntry>& Parent, UFINStruct* Struct, FFINReflectionUIContext* Context) : FFINReflectionUIEntry(Parent, Context), Struct(Struct) {}

TSharedRef<SWidget> FFINReflectionUIStruct::GetDetailsWidget() {
	return SNew(SSplitter) // TODO: Use alternative or rewrite of FINSPlitter
    .PhysicalSplitterHandleSize(25)
    .HitDetectionSplitterHandleSize(25)
    .Style(&Context->Style.Get()->SplitterStyle)
    +SSplitter::Slot().Value(0.7)[
	    SNew(SBox)
		.Padding(FMargin(0, 35, 0, 35))
		.Content()[
            SNew(SScaleBox)
            .Stretch(EStretch::UserSpecified)
            .HAlign(HAlign_Fill)
            .VAlign(VAlign_Fill)
            .UserSpecifiedScale(1.2)[
                SNew(SVerticalBox)
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(5)[
                    GetPreview()
                ]
                +SVerticalBox::Slot()
                .AutoHeight()
                .Padding(5)[
                    SNew(STextBlock)
                    .TextStyle(&Context->Style.Get()->DescriptionTextStyle)
                    .Text(Struct->GetDescription())
                    .AutoWrapText(true)
                ]
                +SVerticalBox::Slot()
                .FillHeight(1)
                .Padding(5)[
                    SNew(SScrollBox)
                    +SScrollBox::Slot().Padding(5)[
                        SNew(SFINReflectionEntryListViewer, &Attributes, Context)
                    ]
                    +SScrollBox::Slot().Padding(5)[
                        SNew(SFINReflectionEntryListViewer, &Functions, Context)
                    ]
                ]
            ]
        ]
    ]
    +SSplitter::Slot().Value(0.3)[
        SNew(SBox)
        .Padding(FMargin(0, 35, 35, 35))
        .MinDesiredWidth(200)
        .Content()[
            SNew(SFINReflectionClassHirachyViewer, SharedThis(this), Context)
            .Style(Context->Style)
        ]
    ];
}

TSharedRef<SWidget> FFINReflectionUIStruct::GetShortPreview() {
	FFINReflectionUIContext* InContext = this->Context;
	return SNew(SHorizontalBox)
    +SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0).VAlign(VAlign_Center)[
        SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->InternalNameTextStyle)
        .Text( FText::FromString(Struct->GetInternalName()))
        .HighlightText_Lambda([InContext]() {
            return FText::FromString(InContext->FilterString);
        })
    ]
    +SHorizontalBox::Slot().AutoWidth().VAlign(EVerticalAlignment::VAlign_Center)[
        SNew(STextBlock)
        .TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
        .Text(Struct->GetDisplayName())
        .HighlightText_Lambda([InContext]() {
            return FText::FromString(InContext->FilterString);
        })
    ];
}

TSharedRef<SWidget> FFINReflectionUIStruct::GetPreview() {
	return GetShortPreview();
}

EFINReflectionFilterState FFINReflectionUIStruct::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	UpdateChildren();
	Filtered.Empty();
	TArray<TSharedPtr<FFINReflectionUIEntry>> Entries;
	Entries.Append(Attributes);
	Entries.Append(Functions);
	for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Entries) {
		if (Entry->ApplyFilter(Filter)) {
			Filtered.Add(Entry);
		}
	}
	if (Filter.PassesFilter(Struct->GetDisplayName().ToString() + " " + Struct->GetInternalName())) return FIN_Ref_Filter_Self;
	if (Filtered.Num() > 0) return FIN_Ref_Filter_Child;
	return FIN_Ref_Filter_None;
}

void FFINReflectionUIClass::LoadChildren() {
	FFINReflectionUIStruct::LoadChildren();

	Signals.Empty();
	for (UFINSignal* Signal : Cast<UFINClass>(Struct)->GetSignals(Context->GetShowRecursive())) {
		Signals.Add(MakeShared<FFINReflectionUISignal>(SharedThis(this), Signal, Context));
	}
}

FFINReflectionUIClass::FFINReflectionUIClass(const TWeakPtr<FFINReflectionUIEntry>& Parent, UFINClass* Class, FFINReflectionUIContext* Context) : FFINReflectionUIStruct(Parent, Class, Context) {}

TSharedRef<SWidget> FFINReflectionUIClass::GetDetailsWidget() {
	return SNew(SSplitter) // TODO: Use alternative or rewrite of FINSplitter
    .PhysicalSplitterHandleSize(25)
    .HitDetectionSplitterHandleSize(25)
    .Style(&Context->Style.Get()->SplitterStyle)
    +SSplitter::Slot()
    .Value(0.7)[
	    SNew(SBox)
		.Padding(FMargin(0, 35, 0, 35))
		.Content()[
			SNew(SScaleBox)
			.Stretch(EStretch::UserSpecified)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.UserSpecifiedScale(1.2)[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)[
					GetPreview()
				]
				+SVerticalBox::Slot()
				.AutoHeight()
				.Padding(5)[
					SNew(STextBlock)
					.TextStyle(&Context->Style.Get()->DescriptionTextStyle)
					.Text(Struct->GetDescription())
					.AutoWrapText(true)
				]
				+SVerticalBox::Slot()
				.FillHeight(1)
				.Padding(5)[
					SNew(SScrollBox)
					+SScrollBox::Slot().Padding(5)[
						SNew(SFINReflectionEntryListViewer, &Attributes, Context)
					]
					+SScrollBox::Slot().Padding(5)[
						SNew(SFINReflectionEntryListViewer, &Functions, Context)
					]
					+SScrollBox::Slot().Padding(5)[
						SNew(SFINReflectionEntryListViewer, &Signals, Context)
					]
				]
			]
		]
	]
	+SSplitter::Slot().Value(0.3)[
        SNew(SBox)
        .Padding(FMargin(0, 35, 35, 35))
        .MinDesiredWidth(200)
        .Content()[
            SNew(SFINReflectionClassHirachyViewer, SharedThis(this), Context)
            .Style(Context->Style)
        ]
	];
}

EFINReflectionFilterState FFINReflectionUIClass::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	EFINReflectionFilterState Passed = FFINReflectionUIStruct::ApplyFilter(Filter);
	for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Signals) {
		if (Entry->ApplyFilter(Filter)) {
			Filtered.Add(Entry);
		}
	}
	if (Passed == FIN_Ref_Filter_None && Filtered.Num() > 0) return FIN_Ref_Filter_Child;
	return Passed;
}

UFINClass* FFINReflectionUIClass::GetClass() const {
	return Cast<UFINClass>(GetStruct());
}

TSharedRef<SWidget> FFINReflectionUIProperty::GetDetailsWidget() {
	return SNew(SBox)
    .Padding(FMargin(0, 35, 35, 35))
    .Content()[
        SNew(SScaleBox)
        .Stretch(EStretch::UserSpecified)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        .UserSpecifiedScale(1.2)[
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)[
                GetPreview()
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
	        .Padding(5)[
                Parent.Pin()->GetLink()
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)[
                SNew(STextBlock)
                .TextStyle(&Context->Style.Get()->DescriptionTextStyle)
                .Text(Property->GetDescription())
                .AutoWrapText(true)
            ]
        ]
    ];
}

TSharedRef<SWidget> FFINReflectionUIProperty::GetShortPreview() {
	FFINReflectionUIContext* InContext = this->Context;
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[
		GeneratePropTypeIcon(Property, InContext)
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0).VAlign(VAlign_Center)[
		GenerateDataTypeIcon(Property, InContext)
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0).VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->InternalNameTextStyle)
		.Text(FText::FromString(Property->GetInternalName()))
		.HighlightText_Lambda([InContext]() {
			return FText::FromString(InContext->FilterString);
		})
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0).VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
		.Text(Property->GetDisplayName())
		.HighlightText_Lambda([InContext]() {
			return FText::FromString(InContext->FilterString);
		})
	];
}
TSharedRef<SWidget> FFINReflectionUIProperty::GetPreview() {
	FFINReflectionUIContext* InContext = this->Context;
	return SNew(SHorizontalBox)
    +SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[
        GeneratePropTypeIcon(Property, InContext)
    ]
    +SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0).VAlign(VAlign_Center)[
        GenerateDataTypeIcon(Property, InContext)
    ]
    +SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0).VAlign(VAlign_Center)[
        SNew(STextBlock)
        .TextStyle(&InContext->Style.Get()->InternalNameTextStyle)
        .Text(FText::FromString(Property->GetInternalName()))
        .HighlightText_Lambda([InContext]() {
            return FText::FromString(InContext->FilterString);
        })
    ]
    +SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0).VAlign(VAlign_Center)[
        SNew(STextBlock)
        .TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
        .Text(Property->GetDisplayName())
        .HighlightText_Lambda([InContext]() {
            return FText::FromString(InContext->FilterString);
        })
    ]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(5, 0, 0, 0)[
		GeneratePropFlags(Property, InContext)
	];
}

EFINReflectionFilterState FFINReflectionUIProperty::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	if (Filter.PassesFilter(Property->GetDisplayName().ToString() + " " + Property->GetInternalName())) return FIN_Ref_Filter_Self;
	return FIN_Ref_Filter_None;
}

void FFINReflectionUIFunction::LoadChildren() {
	FFINReflectionUIEntry::LoadChildren();

	Parameters.Empty();
	for (UFINProperty* Property : Function->GetParameters()) {
		Parameters.Add(MakeShared<FFINReflectionUIProperty>(SharedThis(this), Property, Context));
	}
}

TSharedRef<SWidget> FFINReflectionUIFunction::GetDetailsWidget() {
	UpdateChildren();
	return SNew(SBox)
    .Padding(FMargin(0, 35, 35, 35))
    .Content()[
        SNew(SScaleBox)
        .Stretch(EStretch::UserSpecified)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        .UserSpecifiedScale(1.2)[
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)[
                GetPreview()
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)[
                Parent.Pin()->GetLink()
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)[
                SNew(STextBlock)
                .TextStyle(&Context->Style.Get()->DescriptionTextStyle)
                .Text(Function->GetDescription())
                .AutoWrapText(true)
            ]
            +SVerticalBox::Slot()
            .FillHeight(1)
            .Padding(5)[
                SNew(SScrollBox)
                +SScrollBox::Slot().Padding(5)[
                    SNew(SFINReflectionEntryListViewer, &Parameters, Context)
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> FFINReflectionUIFunction::GetShortPreview() {
	FFINReflectionUIContext* InContext = this->Context;
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		GenerateFuncTypeIcon(Function, InContext)
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->InternalNameTextStyle)
		.Text(FText::FromString(Function->GetInternalName()))
		.HighlightText_Lambda([InContext]() {
			return FText::FromString(InContext->FilterString);
		})
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(5,0,5,0)
	.VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
		.Text(Function->GetDisplayName())
		.HighlightText_Lambda([InContext]() {
			return FText::FromString(InContext->FilterString);
		})
	];
}
TSharedRef<SWidget> FFINReflectionUIFunction::GetPreview() {
	FFINReflectionUIContext* InContext = this->Context;
	TSharedPtr<SHorizontalBox> ParamBox;
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		GenerateFuncTypeIcon(Function, InContext)
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
        SNew(STextBlock)
        .TextStyle(&InContext->Style.Get()->InternalNameTextStyle)
        .Text(FText::FromString(Function->GetInternalName()))
        .HighlightText_Lambda([InContext]() {
            return FText::FromString(InContext->FilterString);
        })
    ]
    +SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(5,0,5,0)
	.VAlign(VAlign_Center)[
        SNew(STextBlock)
        .TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
        .Text(Function->GetDisplayName())
        .HighlightText_Lambda([InContext]() {
            return FText::FromString(InContext->FilterString);
        })
    ]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->ParameterListTextStyle)
		.Text(FText::FromString("("))
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(0,0,0,0)
	.VAlign(VAlign_Center)[
		SAssignNew(ParamBox, SHorizontalBox)
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->ParameterListTextStyle)
		.Text(FText::FromString(")"))
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		GenerateFuncFlags(Function, InContext)
	];
	
	for (UFINProperty* Prop : Function->GetParameters()) {
		if (ParamBox->NumSlots() > 0) {
			ParamBox->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0,0,5,0)[
				SNew(STextBlock)
				.TextStyle(&InContext->Style.Get()->ParameterListTextStyle)
				.Text(FText::FromString(","))
			];
		}
		ParamBox->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)[
				GenerateDataTypeIcon(Prop, InContext)
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(5,0,0,0)[
				SNew(STextBlock)
				.TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
				.Text(Prop->GetDisplayName())
			]
		];
	}
	return Box;
}

EFINReflectionFilterState FFINReflectionUIFunction::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	if (Filter.PassesFilter(Function->GetDisplayName().ToString() + " " + Function->GetInternalName())) return FIN_Ref_Filter_Self;
	return FIN_Ref_Filter_None;
}

void FFINReflectionUISignal::LoadChildren() {
	FFINReflectionUIEntry::LoadChildren();

	Parameters.Empty();
	for (UFINProperty* Property : Signal->GetParameters()) {
		Parameters.Add(MakeShared<FFINReflectionUIProperty>(SharedThis(this), Property, Context));
	}
}

TSharedRef<SWidget> FFINReflectionUISignal::GetDetailsWidget() {
	UpdateChildren();
	return SNew(SBox)
    .Padding(FMargin(0, 35, 35, 35))
    .Content()[
        SNew(SScaleBox)
        .Stretch(EStretch::UserSpecified)
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        .UserSpecifiedScale(1.2)[
            SNew(SVerticalBox)
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)[
                GetPreview()
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)[
                Parent.Pin()->GetLink()
            ]
            +SVerticalBox::Slot()
            .AutoHeight()
            .Padding(5)[
                SNew(STextBlock)
                .TextStyle(&Context->Style.Get()->DescriptionTextStyle)
                .Text(Signal->GetDescription())
                .AutoWrapText(true)
            ]
            +SVerticalBox::Slot()
            .FillHeight(1)
            .Padding(5)[
                SNew(SScrollBox)
                +SScrollBox::Slot().Padding(5)[
                    SNew(SFINReflectionEntryListViewer, &Parameters, Context)
                ]
            ]
        ]
    ];
}

TSharedRef<SWidget> FFINReflectionUISignal::GetShortPreview() {
	FFINReflectionUIContext* InContext = this->Context;
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[
        GenerateSignalTypeIcon(Signal, InContext)
    ]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0).VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->InternalNameTextStyle)
		.Text(FText::FromString(Signal->GetInternalName()))
		.HighlightText_Lambda([InContext]() {
			return FText::FromString(InContext->FilterString);
		})
	]
	+SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
		.Text(Signal->GetDisplayName())
		.HighlightText_Lambda([InContext]() {
			return FText::FromString(InContext->FilterString);
		})
	];
}

TSharedRef<SWidget> FFINReflectionUISignal::GetPreview() {
	FFINReflectionUIContext* InContext = this->Context;
	TSharedPtr<SHorizontalBox> ParamBox;
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		GenerateSignalTypeIcon(Signal, InContext)
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
        SNew(STextBlock)
        .TextStyle(&InContext->Style.Get()->InternalNameTextStyle)
        .Text(FText::FromString(Signal->GetInternalName()))
        .HighlightText_Lambda([InContext]() {
            return FText::FromString(InContext->FilterString);
        })
    ]
    +SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(5,0,5,0)
	.VAlign(VAlign_Center)[
        SNew(STextBlock)
        .TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
        .Text(Signal->GetDisplayName())
        .HighlightText_Lambda([InContext]() {
            return FText::FromString(InContext->FilterString);
        })
    ]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->ParameterListTextStyle)
		.Text(FText::FromString("("))
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(0,0,0,0)
	.VAlign(VAlign_Center)[
		SAssignNew(ParamBox, SHorizontalBox)
	]
	+SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)[
		SNew(STextBlock)
		.TextStyle(&InContext->Style.Get()->ParameterListTextStyle)
		.Text(FText::FromString(")"))
	];
	
	for (UFINProperty* Prop : Signal->GetParameters()) {
		if (ParamBox->NumSlots() > 0) {
			ParamBox->AddSlot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0,0,5,0)[
				SNew(STextBlock)
				.TextStyle(&InContext->Style.Get()->ParameterListTextStyle)
				.Text(FText::FromString(","))
			];
		}
		ParamBox->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)[
				GenerateDataTypeIcon(Prop, InContext)
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(5,0,0,0)[
				SNew(STextBlock)
				.TextStyle(&InContext->Style.Get()->DisplayNameTextStyle)
				.Text(Prop->GetDisplayName())
			]
		];
	}
	return Box;
}

EFINReflectionFilterState FFINReflectionUISignal::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	if (Filter.PassesFilter(Signal->GetDisplayName().ToString() + " " + Signal->GetInternalName())) return FIN_Ref_Filter_Self;
	return FIN_Ref_Filter_None;
}

void FFINReflectionUIContext::SetSelected(FFINReflectionUIEntry* Entry) {
	if (Entry == SelectedEntry) return;
	SelectedEntry = Entry;
	OnSelectionChanged.Broadcast(SelectedEntry);
}

FFINReflectionUIContext::FFINReflectionUIContext() {
	Entries.Empty();
	Structs.Empty();
	for (const TPair<UClass*, UFINClass*>& Class : FFINReflection::Get()->GetClasses()) {
		Structs.Add(Class.Value, MakeShared<FFINReflectionUIClass>(nullptr, Class.Value, this));
	}
	for (const TPair<UScriptStruct*, UFINStruct*>& Struct : FFINReflection::Get()->GetStructs()) {
		Structs.Add(Struct.Value, MakeShared<FFINReflectionUIStruct>(nullptr, Struct.Value, this));
	}
	for (const TPair<UFINStruct*, TSharedPtr<FFINReflectionUIStruct>>& Struct : Structs) Entries.Add(Struct.Value);

	TSharedPtr<FFINReflectionUIStruct>* Struct = Structs.Find(FFINReflection::Get()->FindClass(UObject::StaticClass()));
	if (Struct) SetSelected(Struct->Get());
}

void FFINReflectionUIContext::NavigateNext() {
	if (NavigationHistoryIndex < NavigationHistory.Num() - 1) {
		++NavigationHistoryIndex;
		SetSelected(NavigationHistory[NavigationHistoryIndex]);
	}
}

void FFINReflectionUIContext::NavigatePrevious() {
	if (NavigationHistoryIndex > 0) {
		--NavigationHistoryIndex;
		SetSelected(NavigationHistory[NavigationHistoryIndex]);
	}
}

void FFINReflectionUIContext::NavigateTo(FFINReflectionUIEntry* Entry) {
	if (NavigationHistory.Num() > 0 && NavigationHistory[NavigationHistoryIndex] == Entry) return; 
	if (NavigationHistoryIndex < NavigationHistory.Num() - 1) NavigationHistory.RemoveAt(NavigationHistoryIndex + 1, NavigationHistory.Num() - NavigationHistoryIndex - 1);
	NavigationHistory.Add(Entry);
	if (NavigationHistory.Num() > NAVIGATION_HISTORY_MAX) {
		NavigationHistory.RemoveAt(0);
	}
	NavigationHistoryIndex = NavigationHistory.Num() - 1;
	SetSelected(Entry);
}

FFINReflectionUIEntry* FFINReflectionUIContext::GetSelected() const {
	return SelectedEntry;
}

void FFINReflectionUIContext::SetShowRecursive(bool bInShowRecursive) {
	bShowRecursive = bInShowRecursive;
	for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Entries) {
		Entry->UpdateChildren(true);
	}
	OnSelectionChanged.Broadcast(SelectedEntry);
}
