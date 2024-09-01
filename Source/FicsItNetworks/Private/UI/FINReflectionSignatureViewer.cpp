#include "UI/FINReflectionSignatureViewer.h"
#include "Reflection/FINProperty.h"

void SFINReflectionSignatureViewer::Construct(const FArguments& InArgs, TArray<UFIRProperty*> InSource, FFINReflectionUIContext* InContext) {
	Style = InArgs._Style;
	Context = InContext;
    for (UFIRProperty* Prop : InSource) {
        Source.Add(MakeShared<UFIRProperty*>(Prop));
    }
    
	TSharedPtr<SListView<TSharedPtr<UFIRProperty*>>> List;
	ChildSlot[
        SAssignNew(List, SListView<TSharedPtr<UFIRProperty*>>)
        .ListItemsSource(&Source)
        .ScrollbarVisibility(EVisibility::All)
        .OnGenerateRow_Lambda([this](TSharedPtr<UFIRProperty*> Entry, const TSharedRef<STableViewBase>& Base) {
            return SNew(STableRow<TSharedPtr<UFIRProperty*>>, Base).Content()[
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
