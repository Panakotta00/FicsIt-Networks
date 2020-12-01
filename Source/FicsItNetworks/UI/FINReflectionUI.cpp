#include "FINReflectionUI.h"

#include "STreeView.h"
#include "Reflection/FINReflection.h"

void SFINReflectionUI::Construct(const FArguments& InArgs) {
	Context.Style = InArgs._Style;
	TSharedPtr<SHorizontalBox> Box;
	ChildSlot[
		SAssignNew(Box, SHorizontalBox)
		+SHorizontalBox::Slot().FillWidth(0.3)[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().AutoHeight()[
				SNew(SEditableTextBox)
				.OnTextChanged_Lambda([this](FText Text) {
					FilterCache(Text.ToString());
				})
			]
			+SVerticalBox::Slot().FillHeight(1.0)[
				SNew(SScrollBox)
				.Orientation(EOrientation::Orient_Horizontal)
				+SScrollBox::Slot()[
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
						this->Context.SetSelected(Entry.Get());
					})
				]
			]
		]
	];
	Slot = &Box->AddSlot();
	Slot->FillWidth(1.0);

	Context.OnSelectionChanged.AddLambda([this](FFINReflectionUIEntry* Entry) {
		if (Entry) {
			(*Slot)[Entry->GetDetailsWidget()];
			Tree->SetSelection(SharedThis(Entry));
			TFunction<bool(const TSharedPtr<FFINReflectionUIEntry>&, FFINReflectionUIEntry*)> Func = [&Func, this](const TSharedPtr<FFINReflectionUIEntry>& Check, FFINReflectionUIEntry* Find) {
				bool Found = Check.Get() == Find;
				for (const TSharedPtr<FFINReflectionUIEntry>& Child : Check->GetChildren()) {
					if (Func(Child, Find) && !Found) {
                        Found = true;
					}
				}
				if (Found) Tree->SetItemExpansion(Check, true);
				return Found;
			};
			for (const TSharedPtr<FFINReflectionUIEntry>& FEntry : Filtered) {
				Func(FEntry, Entry);
			}
		}
	});

	FilterCache("");
}

SFINReflectionUI::SFINReflectionUI() {}

void SFINReflectionUI::FilterCache(const FString& InFilter) {
	Context.FilterString = InFilter.TrimStartAndEnd();
	FFINReflectionUIFilter Filter(Context.FilterString);
	Filtered.Empty();
	for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Context.Entries) {
		if (Entry->ApplyFilter(Filter)) Filtered.Add(Entry);
	}
	Tree->RequestTreeRefresh();
	Tree->ClearExpandedItems();
	TFunction<void(const TSharedPtr<FFINReflectionUIEntry>&)> ExpandAll;
	ExpandAll = [this, &ExpandAll](const TSharedPtr<FFINReflectionUIEntry>& Entry) {
		this->Tree->SetItemExpansion(Entry, true);
		for (const TSharedPtr<FFINReflectionUIEntry>& Child : Entry->GetChildren()) {
			ExpandAll(Child);
		}
	};
	if (Context.FilterString.Len() > 0) {
		for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Filtered) {
			ExpandAll(Entry);
		}
	}
}

void UFINReflectionUI::ReleaseSlateResources(bool bReleaseChildren) {
	if (Container) Container.Reset();
}

TSharedRef<SWidget> UFINReflectionUI::RebuildWidget() {
	Container = SNew(SFINReflectionUI).Style_Lambda([this](){ return &Style; });
	return Container.ToSharedRef();
}

void UFINReflectionUI::SetSelected(UFINStruct* InStruct) {
	if (Container) {
		TSharedPtr<FFINReflectionUIStruct>* Entry = Container->Context.Structs.Find(InStruct);
		if (Entry) Container->Context.SetSelected(Entry->Get());
		else Container->Context.SetSelected(nullptr);
	}
}
