#include "FIVSEdActionSelection.h"

void FFIVSEdActionSelectionTextFilter::CallFilterValid(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entries, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)> OnFiltered) {
	OnFiltered(this, Entries, true);
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : Entries->GetChildren()) {
		CallFilterValid(Entry, OnFiltered);
	}
}

FFIVSEdActionSelectionTextFilter::FFIVSEdActionSelectionTextFilter(const FString& Filter) {
    SetFilterText(Filter);
}

bool FFIVSEdActionSelectionTextFilter::Filter(TSharedPtr<FFIVSEdActionSelectionEntry> ToFilter) {
    FString FilterText = ToFilter->GetSignature().SearchableText.ToString().Replace(TEXT(" "), TEXT(""));
    bool bIsValid = true;
    float MatchLength = 0.0f;
    for (const FString& Token : FilterTokens) {
    	if (!FilterText.Contains(Token)) {
    		bIsValid = false;
    	}
    	MatchLength += Token.Len();
    }
    if (bIsValid) {
    	float MatchPercentage = MatchLength / ((float)FilterText.Len());
    	if (BestMatchPercentage < MatchPercentage) {
    		BestMatchPercentage = MatchPercentage;
    		BestMatch = ToFilter;
    	}
    }
    if (bIsValid) {
    	ToFilter->bIsEnabled = true;
    	return true;
    } else {
    	ToFilter->bIsEnabled = false;
    	return false;
    }
}

void FFIVSEdActionSelectionTextFilter::OnFiltered(TSharedPtr<FFIVSEdActionSelectionEntry> Entry, int Pass) {
	Entry->HighlightText = FText::FromString(GetFilterText());
}

void FFIVSEdActionSelectionTextFilter::Reset() {
	BestMatch = nullptr;
	BestMatchPercentage = 0.0f;
}

FString FFIVSEdActionSelectionTextFilter::GetFilterText() {
	FString FilterText;
	for (const FString& Token : FilterTokens) {
		if (FilterText.Len() > 0) FilterText = FilterText.Append(" ");
		FilterText.Append(Token);
	}
	return FilterText;
}

void FFIVSEdActionSelectionTextFilter::SetFilterText(const FString& FilterText) {
	FString TokenList = FilterText;
	FilterTokens.Empty();
	while (TokenList.Len() > 0) {
		FString Token;
		if (TokenList.Split(" ", &Token, &TokenList)) {
			if (Token.Len() > 0) FilterTokens.Add(Token);
		} else {
			FilterTokens.Add(TokenList);
			TokenList = "";
		}
	}
}

TSharedRef<SWidget> FFIVSEdActionSelectionNodeAction::GetTreeWidget() {
	return SNew(STextBlock)
	.Text_Lambda([this]() {
		return NodeAction.Title;
	})
	.HighlightText_Lambda([this](){ return HighlightText; })
	.HighlightColor(FLinearColor(FColor::Yellow))
	.HighlightShape(&HighlightBrush);
}

void FFIVSEdActionSelectionNodeAction::ExecuteAction() {
	UFIVSNode* Node = NewObject<UFIVSNode>(Context.Graph, NodeAction.NodeType);
	Node->Pos = Context.CreationLocation;
	NodeAction.OnExecute.ExecuteIfBound(Node);
	Node->InitPins();
	Context.Graph->AddNode(Node);

	for (UFIVSPin* Pin : Node->GetNodePins()) {
		if (Context.Pin && Pin->CanConnect(Context.Pin)) {
			Pin->AddConnection(Context.Pin);
			break;
		}
	}
}

TSharedRef<SWidget> FFIVSEdActionSelectionCategory::GetTreeWidget() {
	return SNew(STextBlock)
	.Text_Lambda([this]() {
		return Name;
	})
	.HighlightText_Lambda([this](){ return HighlightText; })
    .HighlightColor(FLinearColor(FColor::Yellow))
    .HighlightShape(&HighlightBrush);
}

void SFIVSEdActionSelection::Construct(const FArguments& InArgs) {
	bContextSensitive = InArgs._ContextSensetive.Get();
	OnActionExecuted = InArgs._OnActionExecuted;

	ChildSlot[
		SNew(SBorder)
		.BorderImage(&BackgroundBrush)
		.Content()[
			SNew(SGridPanel)
			+SGridPanel::Slot(0, 0)[
				SNew(STextBlock)
				.Text(FText::FromString("Select Node"))
			]
			+SGridPanel::Slot(1, 0)[
				SNew(SCheckBox)
				.Content()[
					SNew(STextBlock)
					.Text(FText::FromString("Context Sensitive"))
				]]
			+SGridPanel::Slot(0, 1).ColumnSpan(2).Padding(5)[
				SAssignNew(SearchBox, SSearchBox)
				.OnTextChanged_Lambda([this](const FText& Text) {
					TextFilter->SetFilterText(Text.ToString());
					Filter();
					View->RequestTreeRefresh();
					ExpandAll();
					if (TextFilter->BestMatch.IsValid()) SelectRelevant(FindNextRelevant(TextFilter->BestMatch));
				})
			]
			+SGridPanel::Slot(0, 3).ColumnSpan(2)[
				SNew(SBox)
				.MaxDesiredHeight(500)
				.WidthOverride(400)
				.Content()[
					SAssignNew(View, STreeView<TSharedPtr<FFIVSEdActionSelectionEntry>>)
					.OnGenerateRow_Lambda([](TSharedPtr<FFIVSEdActionSelectionEntry> Entry, const TSharedRef<STableViewBase>& Base) {
						return SNew(STableRow<TSharedPtr<FFIVSEdActionSelectionEntry>>, Base).Content()[
							Entry->GetTreeWidget()
						];
					})
					.OnGetChildren_Lambda([this](TSharedPtr<FFIVSEdActionSelectionEntry> Entry, TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>& Childs) {
						if (FilteredChildren.Num() > 0) {
							TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>* Filtered = FilteredChildren.Find(Entry);
							if (Filtered) Childs = *Filtered;
						} else {
							Childs = Entry->GetChildren();
						}
					})
					.TreeItemsSource(&Filtered)
					.OnMouseButtonClick_Lambda([this](TSharedPtr<FFIVSEdActionSelectionEntry> Entry) {
						ExecuteEntry(Entry);
						Close();
					})
				]
			]
		]
	];

	SearchBox->SetOnKeyDownHandler(FOnKeyDown::CreateSP(this, &SFIVSEdActionSelection::OnKeyDown));
}

void SFIVSEdActionSelection::SetFocus() {
	FSlateApplication::Get().SetKeyboardFocus(SearchBox);
}

SFIVSEdActionSelection::SFIVSEdActionSelection() {
	TextFilter = MakeShared<FFIVSEdActionSelectionTextFilter>(TEXT(""));
	Filters.Add(TextFilter);
}

FReply SFIVSEdActionSelection::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Down) {
		SelectNext();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Up) {
		SelectPrevious();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Enter) {
		TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> SelectedItems = View->GetSelectedItems();
		if (SelectedItems.Num() > 0) {
			TSharedPtr<FFIVSEdActionSelectionEntry> Selected = SelectedItems[0];
			if (Selected->IsRelevant()) {
				ExecuteEntry(View->GetSelectedItems()[0]);
				Close();
			}
		}
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

void SFIVSEdActionSelection::SetSource(const TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>& NewSource) {
	Source = NewSource;
	ResetFilters();
}

void SFIVSEdActionSelection::AddSource(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry) {
	Source.Add(Entry);
	ResetFilters();
}

void SFIVSEdActionSelection::ClearSource() {
	Source.Empty();
	ResetFilters();
}

void SFIVSEdActionSelection::ResetFilters() {
	Filtered = Source;
	FilteredChildren.Empty(); 
	for (const TSharedPtr<FFIVSEdActionSelectionFilter>& Filter : Filters) {
		Filter->Reset();
	}
	AllRelevant.Empty();
	TFunction<void(const TSharedPtr<FFIVSEdActionSelectionEntry>&)> AddRelevant;
	AddRelevant = [&AddRelevant, this](const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry) {
		if (Entry->IsRelevant()) AllRelevant.Add(Entry);
		for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Child : Entry->GetChildren()) {
			AddRelevant(Child);
		}
	};
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : Filtered) {
		AddRelevant(Entry);
	}
}

void SFIVSEdActionSelection::SetMenu(const TSharedPtr<IMenu>& inMenu) {
	Menu = inMenu;
}

void SFIVSEdActionSelection::Filter() {
	ResetFilters();
	Filtered.Empty();
	FilteredChildren.Empty();
	AllRelevant.Empty();
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : Source) {
		Filter_Internal(Entry, false);
		if (Entry->bIsEnabled) Filtered.Add(Entry);
	}
}

void SFIVSEdActionSelection::Filter_Internal(TSharedPtr<FFIVSEdActionSelectionEntry> Entry, bool bForceAdd) {
	Entry->bIsEnabled = true;
	bool bBeginForce = true;
	if (!bForceAdd) for (const TSharedPtr<FFIVSEdActionSelectionFilter>& Filter : Filters) {
		bBeginForce = bBeginForce && Filter->Filter(Entry);
	}
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> EnabledChildren;
	for (TSharedPtr<FFIVSEdActionSelectionEntry> Child : Entry->GetChildren()) {
		Filter_Internal(Child, bForceAdd || bBeginForce);
		if (Child->bIsEnabled) EnabledChildren.Add(Child);
	}
	if (EnabledChildren.Num() > 0) {
		FilteredChildren.FindOrAdd(Entry).Append(EnabledChildren);
		Entry->bIsEnabled = true;
	} else if (!bBeginForce) Entry->bIsEnabled = false;
	if (Entry->bIsEnabled && Entry->IsRelevant()) AllRelevant.Add(Entry);
}

void SFIVSEdActionSelection::ExpandAll() {
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : Filtered) {
		View->SetItemExpansion(Entry, true);
	}
	for (TTuple<TSharedPtr<FFIVSEdActionSelectionEntry>, TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>> Entry : FilteredChildren) {
		View->SetItemExpansion(Entry.Key, true);
	}
}

TSharedPtr<FFIVSEdActionSelectionEntry> FindNextChildRelevant(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry) {
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Child : Entry->GetChildren()) {
		if (Entry->IsRelevant()) return Entry;
		TSharedPtr<FFIVSEdActionSelectionEntry> FoundEntry = FindNextChildRelevant(Child);
		if (FoundEntry.IsValid()) return FoundEntry;
	}
	return nullptr;
}

TSharedPtr<FFIVSEdActionSelectionEntry> SFIVSEdActionSelection::FindNextRelevant(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry) {
	if (Entry->IsRelevant()) return Entry;
	return FindNextChildRelevant(Entry);
}

void SFIVSEdActionSelection::SelectRelevant(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry) {
	View->SetSelection(Entry, ESelectInfo::Direct);
}

void SFIVSEdActionSelection::SelectNext() {
	if (AllRelevant.Num() < 1) return;
	int SelectedIndex = -1;
	if (View->GetSelectedItems().Num() > 0) SelectedIndex = AllRelevant.Find(View->GetSelectedItems()[0]);
	++SelectedIndex;
	if (SelectedIndex >= AllRelevant.Num()) SelectedIndex = 0;
	SelectRelevant(AllRelevant[SelectedIndex]);
}

void SFIVSEdActionSelection::SelectPrevious() {
	if (AllRelevant.Num() < 1) return;
	int SelectedIndex = AllRelevant.Num()-1;
	if (View->GetSelectedItems().Num() > 0) SelectedIndex = AllRelevant.Find(View->GetSelectedItems()[0]);
	--SelectedIndex;
	if (SelectedIndex < 0) SelectedIndex = AllRelevant.Num()-1;
	SelectRelevant(AllRelevant[SelectedIndex]);
}

void SFIVSEdActionSelection::Close() {
	if (Menu) {
		FSlateApplication::Get().DismissAllMenus();
	}
}

void SFIVSEdActionSelection::ExecuteEntry(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry) {
	if (dynamic_cast<FFIVSEdActionSelectionAction*>(Entry.Get())) {
		TSharedPtr<FFIVSEdActionSelectionAction> Action = StaticCastSharedPtr<FFIVSEdActionSelectionAction>(Entry);
		Action->ExecuteAction();
		OnActionExecuted.Execute(Action);
	}
}
