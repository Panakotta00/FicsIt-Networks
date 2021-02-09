#include "FINReflectionUI.h"

#include "FINReflectionTreeRow.h"
#include "FINSplitter.h"
#include "Reflection/FINReflection.h"

void SFINReflectionUI::Construct(const FArguments& InArgs) {
	Context.Style = InArgs._Style;
	SSplitter::FSlot* TreeSlot;
	TSharedPtr<SFINSplitter> Box;
	ChildSlot[
		SAssignNew(Box, SFINSplitter)
		.PhysicalSplitterHandleSize(25)
		.HitDetectionSplitterHandleSize(25)
		.Style(&Context.Style.Get()->SplitterStyle)
		+SSplitter::Slot().Expose(TreeSlot)[
			SNew(SVerticalBox)
			+SVerticalBox::Slot().AutoHeight().Padding(10)[
				SAssignNew(SearchBox, SEditableTextBox)
				.Style(&Context.Style.Get()->SearchInputStyle)
				.HintText(FText::FromString("Search for Class/Struct/Property/Function/Signal"))
				.OnTextChanged_Lambda([this](FText Text) {
					FilterCache(Text.ToString());
				})
				.OnTextCommitted_Lambda([this](FText Text, ETextCommit::Type Type) {
					if (this->Filtered.Num() > 0 && Type == ETextCommit::Type::OnEnter) {
						TFunction<bool(const TArray<TSharedPtr<FFINReflectionUIEntry>>&)> CheckChildren;
						FFINReflectionUIFilter Filter(this->Context.FilterString);
						CheckChildren = [this, &Filter, &CheckChildren](const TArray<TSharedPtr<FFINReflectionUIEntry>>& Entries) {
							for (const TSharedPtr<FFINReflectionUIEntry>& Entry : Entries) {
								if (Entry->ApplyFilter(Filter) == FIN_Ref_Filter_Self) {
									this->Context.NavigateTo(Entry.Get());
									return true;
								}
								if (CheckChildren(Entry->GetChildren())) return true;
							}
							return false;
						};
						bool _ = CheckChildren(this->Filtered);
					}
				})
			]
			+SVerticalBox::Slot().FillHeight(1.0)[
				SAssignNew(Tree, STreeView<TSharedPtr<FFINReflectionUIEntry>>)
				.TreeItemsSource(&this->Filtered)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow_Lambda([](TSharedPtr<FFINReflectionUIEntry> Entry, const TSharedRef<STableViewBase>& Base) {
					return SNew(SFINReflectionTreeRow<TSharedPtr<FFINReflectionUIEntry>>, Base)
					.Style(&Entry->Context->Style.Get()->SearchTreeRowStyle)
					.Content()[
						Entry->GetShortPreview()
					];
				})
				.OnGetChildren_Lambda([](TSharedPtr<FFINReflectionUIEntry> Entry, TArray<TSharedPtr<FFINReflectionUIEntry>>& Childs) {
					Childs = Entry->GetChildren();
				})
				.OnSelectionChanged_Lambda([this](TSharedPtr<FFINReflectionUIEntry> Entry, ESelectInfo::Type Type) {
					this->Context.NavigateTo(Entry.Get());
				})
			]
			+SVerticalBox::Slot().AutoHeight()[
				SNew(SCheckBox)
				.Content()[
					SNew(STextBlock)
					.Text(FText::FromString("Show Recursive"))
				]
				.ToolTipText(FText::FromString("Check if all lists of props/funcs/signals of structs/classes should also contain the entries from the parent struct/class."))
				.IsChecked_Lambda([this]() {
					return this->Context.GetShowRecursive() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda([this](ECheckBoxState State) {
					this->Context.SetShowRecursive(State == ECheckBoxState::Checked);
				})
			]
		]
		+SSplitter::Slot().Expose(this->Slot)
	];
	TreeSlot->SizeValue = 0.3f;
	Slot->SizeValue = 1.0f;

	Context.OnSelectionChanged.AddLambda([this](FFINReflectionUIEntry* Entry) {
		if (Entry) {
			(*this->Slot)[Entry->GetDetailsWidget()];
			this->Tree->SetSelection(SharedThis(Entry));
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
			for (const TSharedPtr<FFINReflectionUIEntry>& FEntry : this->Filtered) {
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

FReply SFINReflectionUI::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) {
	SCompoundWidget::OnFocusReceived(MyGeometry, InFocusEvent);

	return FReply::Unhandled().SetUserFocus(SearchBox.ToSharedRef());
}

FReply SFINReflectionUI::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	FKey Key = InKeyEvent.GetKey();
	if (!InKeyEvent.GetModifierKeys().IsAltDown() && !InKeyEvent.GetModifierKeys().IsControlDown() && Key != EKeys::Left && Key != EKeys::Up && Key != EKeys::Right && Key != EKeys::Down && Key != EKeys::Tab) {
		SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
		return FReply::Unhandled().SetUserFocus(SearchBox.ToSharedRef());
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SFINReflectionUI::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
	if (MouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton) {
		Context.NavigatePrevious();
		return FReply::Handled();
	}
	if (MouseEvent.GetEffectingButton() == EKeys::ThumbMouseButton2) {
		Context.NavigateNext();
		return FReply::Handled();
	}

	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

bool SFINReflectionUI::IsInteractable() const {
	return true;
}

bool SFINReflectionUI::SupportsKeyboardFocus() const {
	return true;
}

void SFINReflectionUI::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) {
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (!FSlateApplication::Get().GetKeyboardFocusedWidget().Get() || bInitFocus) {
		bInitFocus = false;
		FSlateApplication::Get().SetAllUserFocus(SearchBox);
	}
}

void UFINReflectionUI::ReleaseSlateResources(bool bReleaseChildren) {
	if (Container) Container.Reset();
}

TSharedRef<SWidget> UFINReflectionUI::RebuildWidget() {
	Container = SNew(SFINReflectionUI).Style_Lambda([this](){ return &Style; });
	return Container.ToSharedRef();
}

void UFINReflectionUI::NavigateTo(UFINStruct* InStruct) {
	if (Container) {
		TSharedPtr<FFINReflectionUIStruct>* Entry = Container->Context.Structs.Find(InStruct);
		if (Entry) Container->Context.NavigateTo(Entry->Get());
		else Container->Context.NavigateTo(nullptr);
	}
}

void UFINReflectionUI::SetShowRecursive(bool bInShowRecursive) {
	if (Container) {
		Container->Context.SetShowRecursive(bInShowRecursive);
	}
}
