#include "UI/FINReflectionClassHirachyViewer.h"

#include "Reflection/FIRStruct.h"
#include "UI/FINReflectionTreeRow.h"

void SFINReflectionClassHirachyViewer::Construct(const FArguments& InArgs, const TSharedPtr<FFINReflectionUIStruct>& InSearchStruct, FFINReflectionUIContext* InContext) {
	Style = InArgs._Style;
	Context = InContext;
	SearchStruct = InSearchStruct;
	
	UFIRStruct* Outer = SearchStruct->GetStruct();
	// Find upper most parent of struct
	while (Outer->GetParent()) {
		Outer = Outer->GetParent();
	}
	
	StructSource.Add(*Context->Structs.Find(Outer));
	
	TSharedPtr<STreeView<TSharedPtr<FFINReflectionUIStruct>>> Tree;
	ChildSlot[
		SAssignNew(Tree, STreeView<TSharedPtr<FFINReflectionUIStruct>>)
		.SelectionMode(ESelectionMode::Single)
		.TreeItemsSource(&StructSource)
		.OnGenerateRow_Lambda([this](TSharedPtr<FFINReflectionUIStruct> Entry, const TSharedRef<STableViewBase>& Base) {
			return SNew(SFINReflectionTreeRow<TSharedPtr<FFINReflectionUIStruct>>, Base)
			.Style(&Context->Style.Get()->HirachyTreeRowStyle)
			.Content()[
				Entry->GetShortPreview()
			];
		})
		.OnGetChildren_Lambda([this](TSharedPtr<FFINReflectionUIStruct> InEntry, TArray<TSharedPtr<FFINReflectionUIStruct>>& OutArray) {
			OutArray.Empty();
			TArray<UFIRStruct*> Children = InEntry->GetStruct()->GetChildren();
			for (UFIRStruct* Struct : Children) {
				TSharedPtr<FFINReflectionUIStruct>* Child = Context->Structs.Find(Struct);
				if (Child) {
					if (InEntry != SearchStruct) {
						if (!SearchStruct->GetStruct()->IsChildOf(Child->Get()->GetStruct())) continue;
					}
					OutArray.Add(*Child);
				}
			}
		})
		.OnMouseButtonDoubleClick_Lambda([this](TSharedPtr<FFINReflectionUIStruct> Entry) {
			this->Context->NavigateTo(Entry.Get());
		})
	];
	for (const TPair<UFIRStruct*, TSharedPtr<FFINReflectionUIStruct>>& Entry : Context->Structs) {
		Tree->SetItemExpansion(Entry.Value, true);
	}
	Tree->SetSelection(SearchStruct);
}
