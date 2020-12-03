#include "FINReflectionUIContext.h"

#include "FINReflectionClassHirachyViewer.h"
#include "FINReflectionEntryListViewer.h"
#include "FINReflectionSignatureViewer.h"
#include "Reflection/FINReflection.h"
#include "Network/FINNetworkValues.h"
#include "Reflection/FINArrayProperty.h"
#include "Reflection/FINClassProperty.h"
#include "Reflection/FINObjectProperty.h"
#include "Reflection/FINStructProperty.h"
#include "Reflection/FINTraceProperty.h"

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
	switch (Prop->GetType()) {
	case FIN_OBJ: {
		UFINClass* Class = FFINReflection::Get()->FindClass(Cast<UFINObjectProperty>(Prop)->GetSubclass());
		if (!Class) break;
		return SNew(SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString("Obj("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(Class->GetDisplayName())
            .OnDoubleClicked_Lambda([Context, Class]() {
                Context->SetSelected(Context->Structs.Find(Class)->Get());
                return FReply::Handled();
            })]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString(")"))
        ]];
	} case FIN_CLASS: {
		UFINClass* Class = FFINReflection::Get()->FindClass(Cast<UFINClassProperty>(Prop)->GetSubclass());
		if (!Class) break;
		return SNew(SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString("Class("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(Class->GetDisplayName())
            .OnDoubleClicked_Lambda([Context, Class]() {
                Context->SetSelected(Context->Structs.Find(Class)->Get());
                return FReply::Handled();
            })]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString(")"))
        ]];
	} case FIN_TRACE: {
		UFINClass* Class = FFINReflection::Get()->FindClass(Cast<UFINTraceProperty>(Prop)->GetSubclass());
		if (!Class) break;
		return SNew(SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString("Trace("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(Class->GetDisplayName())
            .OnDoubleClicked_Lambda([Context, Class]() {
                Context->SetSelected(Context->Structs.Find(Class)->Get());
                return FReply::Handled();
            })]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString(")"))
        ]];
	} case FIN_STRUCT: {
		UFINStruct* Struct = FFINReflection::Get()->FindStruct(Cast<UFINStructProperty>(Prop)->GetInner());
		if (!Struct) break;
		return SNew(SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString("Struct("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(Struct->GetDisplayName())
            .OnDoubleClicked_Lambda([Context, Struct]() {
                Context->SetSelected(Context->Structs.Find(Struct)->Get());
                return FReply::Handled();
            })]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString(")"))
        ]];
	} case FIN_ARRAY: {
		return SNew(SBox).Content()[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString("Array("))
        ]
        +SHorizontalBox::Slot().AutoWidth()[
			GenerateDataTypeIcon(Cast<UFINArrayProperty>(Prop)->GetInnerType(), Context)
		]
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock)
            .Text(FText::FromString(")"))
        ]];
	} default: ; }

	FString Text = GetText(Prop);
	return SNew(STextBlock).Text(FText::FromString(Text));
}

TSharedRef<SWidget> GeneratePropTypeIcon(UFINProperty* Prop, FFINReflectionUIContext* Context) {
	const FFINReflectionUIStyleStruct* Style = Context->Style.Get();
	check(Style != nullptr);
	TSharedRef<SImage> Image = SNew(SImage).Image(&Style->MemberAttrib);
	if (Prop->GetPropertyFlags() & FIN_Prop_ClassProp) Image->SetImage(&Style->ClassAttrib);
	return SNew(SBox).WidthOverride(15).HeightOverride(15)[
		Image
	];
}

TSharedRef<SWidget> GenerateFuncTypeIcon(UFINFunction* Func, FFINReflectionUIContext* Context) {
	const FFINReflectionUIStyleStruct* Style = Context->Style.Get();
	check(Style != nullptr);
	TSharedRef<SImage> Image = SNew(SImage).Image(&Style->MemberFunc);
	if (Func->GetFunctionFlags() & FIN_Func_ClassFunc) Image->SetImage(&Style->ClassFunc);
	if (Func->GetFunctionFlags() & FIN_Func_StaticFunc) Image->SetImage(&Style->StaticFunc);
	return SNew(SBox).WidthOverride(15).HeightOverride(15).Content()[
		Image
	];
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

FFINReflectionUIStruct::FFINReflectionUIStruct(UFINStruct* Struct, FFINReflectionUIContext* Context) : FFINReflectionUIEntry(Context), Struct(Struct) {
	for (UFINProperty* Prop : Struct->GetProperties()) {
		Attributes.Add(MakeShared<FFINReflectionUIProperty>(Prop, Context));
	}
	for (UFINFunction* Func : Struct->GetFunctions()) {
		Functions.Add(MakeShared<FFINReflectionUIFunction>(Func, Context));
	}
	Filtered = Attributes;
	Filtered.Append(Functions);
}

TSharedRef<SWidget> FFINReflectionUIStruct::GetDetailsWidget() {
	return SNew(SGridPanel)
    .FillColumn(0, 1)
    .FillRow(1, 1)
    +SGridPanel::Slot(0, 0)[
        GetPreview()
    ]
    +SGridPanel::Slot(0,1)[
        SNew(SScrollBox)
        +SScrollBox::Slot()[
            SNew(STextBlock).Text(Struct->GetDescription())
        ]
        +SScrollBox::Slot()[
            SNew(SFINReflectionEntryListViewer, &Attributes, Context)
        ]
        +SScrollBox::Slot()[
            SNew(SFINReflectionEntryListViewer, &Functions, Context)
        ]
    ]
    +SGridPanel::Slot(1, 1)[
        SNew(SBox)
        .MinDesiredWidth(200)
        .Content()[
            SNew(SFINReflectionClassHirachyViewer, SharedThis(this), Context)
            .Style(Context->Style)
        ]
    ];
}

TSharedRef<SWidget> FFINReflectionUIStruct::GetShortPreview() {
	return SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[
            SNew(STextBlock).Text(Struct->GetDisplayName())
            .HighlightText_Lambda([this](){ return FText::FromString(Context->FilterString); })
        ]
        +SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0)[
            SNew(STextBlock).Text( FText::FromString(Struct->GetInternalName()))
            .HighlightText_Lambda([this](){ return FText::FromString(Context->FilterString); })
        ];
}

TSharedRef<SWidget> FFINReflectionUIStruct::GetPreview() {
	return GetShortPreview();
}

bool FFINReflectionUIStruct::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	Filtered.Empty();
	TArray<TSharedPtr<FFINReflectionUIEntry>> Entries;
	Entries.Append(Attributes);
	Entries.Append(Functions);
	for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Entries) {
		if (Entry->ApplyFilter(Filter)) {
			Filtered.Add(Entry);
		}
	}
	return Filtered.Num() > 0 || Filter.PassesFilter(Struct->GetDisplayName().ToString() + " " + Struct->GetInternalName());
}

FFINReflectionUIClass::FFINReflectionUIClass(UFINClass* Class, FFINReflectionUIContext* Context) : FFINReflectionUIStruct(Class, Context) {
	for (UFINRefSignal* Signal : Class->GetSignals()) {
		Signals.Add(MakeShared<FFINReflectionUISignal>(Signal, Context));
	}
	Filtered.Append(Signals);
}

TSharedRef<SWidget> FFINReflectionUIClass::GetDetailsWidget() {
	return SNew(SGridPanel)
	.FillColumn(0, 1)
	.FillRow(1, 1)
	+SGridPanel::Slot(0, 0)[
		GetPreview()
	]
	+SGridPanel::Slot(0,1)[
		SNew(SScrollBox)
		+SScrollBox::Slot()[
			SNew(STextBlock).Text(Struct->GetDescription())
		]
		+SScrollBox::Slot()[
			SNew(SFINReflectionEntryListViewer, &Attributes, Context)
		]
		+SScrollBox::Slot()[
			SNew(SFINReflectionEntryListViewer, &Functions, Context)
		]
		+SScrollBox::Slot()[
			SNew(SFINReflectionEntryListViewer, &Signals, Context)
		]
	]
	+SGridPanel::Slot(1, 1)[
		SNew(SBox)
		.MinDesiredWidth(200)
		.Content()[
			SNew(SFINReflectionClassHirachyViewer, SharedThis(this), Context)
			.Style(Context->Style)
		]
	];
}

bool FFINReflectionUIClass::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	bool Passed = FFINReflectionUIStruct::ApplyFilter(Filter);
	for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Signals) {
		if (Entry->ApplyFilter(Filter)) {
			Filtered.Add(Entry);
		}
	}
	return Passed || Filtered.Num() > 0;
}

UFINClass* FFINReflectionUIClass::GetClass() const {
	return Cast<UFINClass>(GetStruct());
}

TSharedRef<SWidget> FFINReflectionUIProperty::GetDetailsWidget() {
	return SNew(SGridPanel)
	.FillColumn(0, 1)
	.FillRow(1, 1)
	+SGridPanel::Slot(0, 0)[
		GetPreview()
	]
	+SGridPanel::Slot(0,1)[
		SNew(SScrollBox)
		+SScrollBox::Slot()[
			SNew(STextBlock).Text( Property->GetDescription())
		]
	];
}

TSharedRef<SWidget> FFINReflectionUIProperty::GetShortPreview() {
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot().AutoWidth()[
		GeneratePropTypeIcon(Property, Context)
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0)[
		GenerateDataTypeIcon(Property, Context)
	]
	+SHorizontalBox::Slot().AutoWidth()[
		SNew(STextBlock).Text(Property->GetDisplayName())
		.HighlightText_Lambda([this](){ return FText::FromString(Context->FilterString); })
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0)[
		SNew(STextBlock).Text(FText::FromString(Property->GetInternalName()))
		.HighlightText_Lambda([this](){ return FText::FromString(Context->FilterString); })
	];
}
TSharedRef<SWidget> FFINReflectionUIProperty::GetPreview() {
	return GetShortPreview();
}

bool FFINReflectionUIProperty::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	return Filter.PassesFilter(Property->GetDisplayName().ToString() + " " + Property->GetInternalName());
}

TSharedRef<SWidget> FFINReflectionUIFunction::GetDetailsWidget() {
	TSharedRef<SGridPanel> Panel = SNew(SGridPanel)
	.FillColumn(0, 1)
	.FillRow(1, 1)
	+SGridPanel::Slot(0, 0)[
		GetPreview()
	]
	+SGridPanel::Slot(0,1)[
		SNew(SScrollBox)
		+SScrollBox::Slot()[
			SNew(STextBlock).Text(Function->GetDescription())
		]
		+SScrollBox::Slot()[
			SNew(SFINReflectionSignatureViewer, Function->GetParameters(), Context)
			.Style(Context->Style)
		]
	];
	TSharedPtr<FFINReflectionUIStruct>* Struct = Context->Structs.Find(Cast<UFINStruct>(Function->GetOuter()));
	SGridPanel::FSlot& Slot = Panel->AddSlot(1, 0);
	if (Struct && Struct->IsValid()) Slot[
		Struct->Get()->GetShortPreview()
	];
	return Panel;
}

TSharedRef<SWidget> FFINReflectionUIFunction::GetShortPreview() {
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot().AutoWidth()[
		GenerateFuncTypeIcon(Function, Context)
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0)[
		SNew(STextBlock).Text(Function->GetDisplayName())
		.HighlightText_Lambda([this](){ return FText::FromString(Context->FilterString); })
	]
	+SHorizontalBox::Slot().AutoWidth()[
		SNew(STextBlock).Text(FText::FromString(Function->GetInternalName()))
		.HighlightText_Lambda([this](){ return FText::FromString(Context->FilterString); })
	];
}
TSharedRef<SWidget> FFINReflectionUIFunction::GetPreview() {
	TSharedPtr<SHorizontalBox> ParamBox;
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+SHorizontalBox::Slot().AutoWidth()[
		GenerateFuncTypeIcon(Function, Context)
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0)[
		SNew(STextBlock).Text(Function->GetDisplayName())
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)[
		SNew(STextBlock).Text(FText::FromString(Function->GetInternalName()))
	]
	+SHorizontalBox::Slot().AutoWidth()[
		SNew(STextBlock).Text(FText::FromString("("))
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0)[
		SAssignNew(ParamBox, SHorizontalBox)
	]
	+SHorizontalBox::Slot().AutoWidth()[
		SNew(STextBlock).Text(FText::FromString(")"))
	];
	for (UFINProperty* Prop : Function->GetParameters()) {
		if (ParamBox->NumSlots() > 0) {
			ParamBox->AddSlot().AutoWidth().Padding(5,0,5,0)[
				SNew(STextBlock).Text(FText::FromString(","))
			];
		}
		ParamBox->AddSlot().AutoWidth()[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth()[
				GenerateDataTypeIcon(Prop, Context)
			]
			+SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0)[
				SNew(STextBlock).Text(Prop->GetDisplayName())
			]
		];
	}
	return Box;
}

bool FFINReflectionUIFunction::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	return Filter.PassesFilter(Function->GetDisplayName().ToString() + " " + Function->GetInternalName());
}

TSharedRef<SWidget> FFINReflectionUISignal::GetDetailsWidget() {
	return SNew(SGridPanel)
	.FillColumn(0, 1)
	.FillRow(1, 1)
	+SGridPanel::Slot(0, 0)[
		GetPreview()
	]
	+SGridPanel::Slot(0,1)[
		SNew(SScrollBox)
		+SScrollBox::Slot()[
			SNew(STextBlock).Text(Signal->GetDescription())
		]
		+SScrollBox::Slot()[
			SNew(SFINReflectionSignatureViewer, Signal->GetParameters(), Context)
			.Style(Context->Style)
		]
	];
}

TSharedRef<SWidget> FFINReflectionUISignal::GetShortPreview() {
	return SNew(SHorizontalBox)
	+SHorizontalBox::Slot().AutoWidth()[
		SNew(STextBlock).Text(Signal->GetDisplayName())
		.HighlightText_Lambda([this](){ return FText::FromString(Context->FilterString); })
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0)[
		SNew(STextBlock).Text(FText::FromString(Signal->GetInternalName()))
		.HighlightText_Lambda([this](){ return FText::FromString(Context->FilterString); })
	];
}

TSharedRef<SWidget> FFINReflectionUISignal::GetPreview() {
	TSharedPtr<SHorizontalBox> ParamBox;
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox)
	+SHorizontalBox::Slot().AutoWidth()[
		SNew(STextBlock).Text(Signal->GetDisplayName())
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0)[
		SNew(STextBlock).Text(FText::FromString(Signal->GetInternalName()))
	]
	+SHorizontalBox::Slot().AutoWidth()[
		SNew(STextBlock).Text(FText::FromString("("))
	]
	+SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0)[
		SAssignNew(ParamBox, SHorizontalBox)
	]
	+SHorizontalBox::Slot().AutoWidth()[
		SNew(STextBlock).Text(FText::FromString(")"))
	];
	for (UFINProperty* Prop : Signal->GetParameters()) {
		if (ParamBox->NumSlots() > 0) {
			ParamBox->AddSlot().AutoWidth().Padding(5,0,5,0)[
				SNew(STextBlock).Text(FText::FromString(","))
			];
		}
		ParamBox->AddSlot().AutoWidth()[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot().AutoWidth()[
				GenerateDataTypeIcon(Prop, Context)
			]
			+SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0)[
				SNew(STextBlock).Text(Prop->GetDisplayName())
			]
		];
	}
	return Box;
}

bool FFINReflectionUISignal::ApplyFilter(const FFINReflectionUIFilter& Filter) {
	return Filter.PassesFilter(Signal->GetDisplayName().ToString() + " " + Signal->GetInternalName());
}

FFINReflectionUIContext::FFINReflectionUIContext() {
	Entries.Empty();
	Structs.Empty();
	for (const TPair<UClass*, UFINClass*>& Class : FFINReflection::Get()->GetClasses()) {
		UE_LOG(LogTemp, Warning, TEXT("%p %s %p %s"), Class.Key, *Class.Key->GetName(), Class.Value, *Class.Value->GetInternalName());
		Structs.Add(Class.Value, MakeShared<FFINReflectionUIClass>(Class.Value, this));
	}
	for (const TPair<UScriptStruct*, UFINStruct*>& Struct : FFINReflection::Get()->GetStructs()) {
		UE_LOG(LogTemp, Warning, TEXT("%p %s %p %s"), Struct.Key, *Struct.Key->GetName(), Struct.Value, *Struct.Value->GetInternalName());
		Structs.Add(Struct.Value, MakeShared<FFINReflectionUIStruct>(Struct.Value, this));
	}
	for (const TPair<UFINStruct*, TSharedPtr<FFINReflectionUIStruct>>& Struct : Structs) Entries.Add(Struct.Value);
}
