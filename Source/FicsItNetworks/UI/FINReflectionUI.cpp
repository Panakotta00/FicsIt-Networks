#include "FINReflectionUI.h"

#include "STreeView.h"
#include "Reflection/FINReflection.h"

void SFINReflectionUI::Construct(const FArguments& InArgs) {
	Context.Style = InArgs._Style;
	TSharedPtr<SHorizontalBox> Box;
	ChildSlot[
        SAssignNew(Box, SHorizontalBox)
        +SHorizontalBox::Slot()[
            SNew(SVerticalBox)
            +SVerticalBox::Slot().AutoHeight()[
                SNew(SEditableTextBox)
                .OnTextChanged_Lambda([this](FText Text) {
	                FilterCache(Text.ToString());
                })
            ]
            +SVerticalBox::Slot().AutoHeight()[
	            SAssignNew(Tree, STreeView<TSharedPtr<FFINReflectionUIEntry>>)
	            .TreeItemsSource(&Filtered)
	            .OnGenerateRow_Lambda([](TSharedPtr<FFINReflectionUIEntry> Entry, const TSharedRef<STableViewBase>& Base) {
	                return SNew(STableRow<TSharedPtr<FFINReflectionUIEntry>>, Base).Content()[
	                    Entry->GetShortPreview()
	                ];
	            })
	            .OnGetChildren_Lambda([](TSharedPtr<FFINReflectionUIEntry> Entry, TArray<TSharedPtr<FFINReflectionUIEntry>>& Childs) {
	                Childs = Entry->GetChildren();
	            })
	            .OnSelectionChanged_Lambda([this](TSharedPtr<FFINReflectionUIEntry> Entry, ESelectInfo::Type Type) {
	                Context.SetSelected(Entry.Get());
	            })
			]
		]
	];
	Slot = &Box->AddSlot();

	Context.OnSelectionChanged.AddLambda([this](FFINReflectionUIEntry* Entry) {
		if (Entry) {
			(*Slot)[Entry->GetDetailsWidget()];
			Tree->SetSelection(SharedThis(Entry));
		}
	});

	FilterCache("");
}

SFINReflectionUI::SFINReflectionUI() {}

void SFINReflectionUI::FilterCache(const FString& InFilter) {
	Context.FilterString = InFilter;
	FFINReflectionUIFilter Filter(InFilter);
	Filtered.Empty();
	for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Context.Entries) {
		if (Entry->ApplyFilter(Filter)) Filtered.Add(Entry);
	}
	Tree->RequestTreeRefresh();
}

void UFINReflectionUI::ReleaseSlateResources(bool bReleaseChildren) {
	if (Container) Container.Reset();
}

TSharedRef<SWidget> UFINReflectionUI::RebuildWidget() {
	Container = SNew(SFINReflectionUI).Style_Lambda([this](){ return &Style; });
	return Container.ToSharedRef();
}
