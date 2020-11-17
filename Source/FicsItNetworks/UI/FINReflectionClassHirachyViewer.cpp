#include "FINReflectionClassHirachyViewer.h"

#include "Reflection/FINReflection.h"

void SFINReflectionClassHirachyViewer::Construct(const FArguments& InArgs, const TSharedPtr<FFINReflectionUIClass>& InSearchClass, FFINReflectionUIContext* InContext) {
	Style = InArgs._Style;
	Context = InContext;
	SearchClass = InSearchClass;
	ClassSource.Add(*Context->Classes.Find(FFINReflection::Get()->FindClass(UObject::StaticClass())));
	TSharedPtr<STreeView<TSharedPtr<FFINReflectionUIClass>>> Tree;
	ChildSlot[
		SAssignNew(Tree, STreeView<TSharedPtr<FFINReflectionUIClass>>)
		.TreeItemsSource(&ClassSource)
		.OnGenerateRow_Lambda([](TSharedPtr<FFINReflectionUIClass> Entry, const TSharedRef<STableViewBase>& Base) {
			return SNew(STableRow<TSharedPtr<FFINReflectionUIClass>>, Base).Content()[
				Entry->GetShortPreview()
			];
		})
		.OnGetChildren_Lambda([this](TSharedPtr<FFINReflectionUIClass> InEntry, TArray<TSharedPtr<FFINReflectionUIClass>>& OutArray) {
			OutArray.Empty();
			TArray<UFINClass*> Children = InEntry->GetClass()->GetChildClasses();
			for (UFINClass* Class : Children) {
				TSharedPtr<FFINReflectionUIClass>* Child = Context->Classes.Find(Class);
				if (Child) {
					if (InEntry != SearchClass) {
						if (!SearchClass->GetClass()->IsChildOf(Child->Get()->GetClass())) continue;
					}
					OutArray.Add(*Child);
				}
			}
		})
		.OnMouseButtonDoubleClick_Lambda([this](TSharedPtr<FFINReflectionUIClass> Entry) {
			Context->SetSelected(Entry.Get());
		})
	];
	for (const TPair<UFINClass*, TSharedPtr<FFINReflectionUIClass>>& Entry : Context->Classes) {
		Tree->SetItemExpansion(Entry.Value, true);
	}
	Tree->SetSelection(SearchClass);
}
