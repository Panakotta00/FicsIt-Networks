#include "FINReflectionEntryListViewer.h"

void SFINReflectionEntryListViewer::Construct(const FArguments& InArgs, TArray<TSharedPtr<FFINReflectionUIEntry>>* InSource, FFINReflectionUIContext* InContext) {
	Style = InArgs._Style;
	Context = InContext;
	Source = InSource;
	ChildSlot[
		SAssignNew(List, SListView<TSharedPtr<FFINReflectionUIEntry>>)
		.ListItemsSource(Source)
		.SelectionMode(ESelectionMode::Single)
		.OnGenerateRow_Lambda([this](TSharedPtr<FFINReflectionUIEntry> Entry, const TSharedRef<STableViewBase>& Base) {
			if (!Entry.IsValid()) return SNew(STableRow<TSharedPtr<FFINReflectionUIEntry>>, Base);
			return SNew(STableRow<TSharedPtr<FFINReflectionUIEntry>>, Base)
			.Padding(FMargin(5, 0, 5, 0))
			.Style(&Context->Style.Get()->EntryListRowStyle)
			.Content()[
				Entry->GetPreview()
			];
		})
		.OnMouseButtonDoubleClick_Lambda([this](TSharedPtr<FFINReflectionUIEntry> Entry) {
			this->Context->NavigateTo(Entry.Get());
		})
	];

	Context->OnSelectionChanged.AddSP(SharedThis(this), &SFINReflectionEntryListViewer::UpdateList);
}

FReply SFINReflectionEntryListViewer::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Enter || InKeyEvent.GetKey() == EKeys::SpaceBar) {
		if (List->GetSelectedItems().Num() > 0) {
			Context->NavigateTo(List->GetSelectedItems()[0].Get());
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

void SFINReflectionEntryListViewer::OnFocusChanging(const FWeakWidgetPath& PreviousFocusPath, const FWidgetPath& NewWidgetPath, const FFocusEvent& InFocusEvent) {
	if (NewWidgetPath.GetLastWidget() == List) {
		if (List->GetSelectedItems().Num() < 1 && Source->Num() > 0) {
			List->SetItemSelection((*Source)[0], true);
		}
	}
}

void SFINReflectionEntryListViewer::UpdateList(FFINReflectionUIEntry* Entry) {
	List->RequestListRefresh();
}
