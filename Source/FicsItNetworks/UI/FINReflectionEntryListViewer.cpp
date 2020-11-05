#include "FINReflectionEntryListViewer.h"

void SFINReflectionEntryListViewer::Construct(const FArguments& InArgs, TArray<TSharedPtr<FFINReflectionUIEntry>>* InSource, FFINReflectionUIContext* InContext) {
	Style = InArgs._Style;
	Context = InContext;
	Source = InSource;
	TSharedPtr<SListView<TSharedPtr<FFINReflectionUIEntry>>> List;
	ChildSlot[
        SAssignNew(List, SListView<TSharedPtr<FFINReflectionUIEntry>>)
        .ListItemsSource(Source)
        .OnGenerateRow_Lambda([](TSharedPtr<FFINReflectionUIEntry> Entry, const TSharedRef<STableViewBase>& Base) {
            return SNew(STableRow<TSharedPtr<FFINReflectionUIEntry>>, Base).Content()[
                Entry->GetPreview()
            ];
        })
        .OnMouseButtonDoubleClick_Lambda([this](TSharedPtr<FFINReflectionUIEntry> Entry) {
            Context->SetSelected(Entry.Get());
        })
    ];
}
