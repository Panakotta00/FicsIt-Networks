#include "FINReflectionSignatureViewer.h"
#include "Reflection/FINReflection.h"

void SFINReflectionSignatureViewer::Construct(const FArguments& InArgs, TArray<UFINProperty*> InSource, FFINReflectionUIContext* InContext) {
	Style = InArgs._Style;
	Context = InContext;
    for (UFINProperty* Prop : InSource) {
        Source.Add(MakeShared<UFINProperty*>(Prop));
    }
    
	TSharedPtr<SListView<TSharedPtr<UFINProperty*>>> List;
	ChildSlot[
        SAssignNew(List, SListView<TSharedPtr<UFINProperty*>>)
        .ListItemsSource(&Source)
        .ScrollbarVisibility(EVisibility::All)
        .OnGenerateRow_Lambda([this](TSharedPtr<UFINProperty*> Entry, const TSharedRef<STableViewBase>& Base) {
            return SNew(STableRow<TSharedPtr<UFINProperty*>>, Base).Content()[
                SNew(SVerticalBox)
                +SVerticalBox::Slot().AutoHeight()[
                    SNew(SHorizontalBox)
                    +SHorizontalBox::Slot().AutoWidth().Padding(5,0,5,0)[
                        GenerateDataTypeIcon(*Entry, Context)
                    ]
                    +SHorizontalBox::Slot().AutoWidth()[
                        SNew(STextBlock).Text((*Entry)->GetDisplayName())
                    ]
                    +SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0)[
                        SNew(STextBlock).Text(FText::FromString((*Entry)->GetInternalName()))
                    ]
                ]
                +SVerticalBox::Slot().AutoHeight()[
                    SNew(STextBlock).Text((*Entry)->GetDescription())
                ]
            ];
        })
    ];
}
