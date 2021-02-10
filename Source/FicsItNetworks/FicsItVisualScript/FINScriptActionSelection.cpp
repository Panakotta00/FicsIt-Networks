#include "FINScriptActionSelection.h"

#include "SSearchBox.h"

void FFINScriptActionSelectionTextFilter::CallFilterValid(const TSharedPtr<FFINScriptActionSelectionEntry>& Entries, TFunction<void(FFINScriptActionSelectionFilter*, const TSharedPtr<FFINScriptActionSelectionEntry>&, bool)> OnFiltered) {
	OnFiltered(this, Entries, true);
	for (const TSharedPtr<FFINScriptActionSelectionEntry>& Entry : Entries->GetChilds()) {
		CallFilterValid(Entry, OnFiltered);
	}
}

FFINScriptActionSelectionTextFilter::FFINScriptActionSelectionTextFilter(const FString& Filter) {
    SetFilterText(Filter);
}

TArray<TSharedPtr<FFINScriptActionSelectionEntry>> FFINScriptActionSelectionTextFilter::Filter(const TArray<TSharedPtr<FFINScriptActionSelectionEntry>>& ToFilter, TFunction<void(FFINScriptActionSelectionFilter*, const TSharedPtr<FFINScriptActionSelectionEntry>&, bool)> OnFiltered) {
	TArray<TSharedPtr<FFINScriptActionSelectionEntry>> Filtered;
    for (const TSharedPtr<FFINScriptActionSelectionEntry>& Entry : ToFilter) {
    	FString FilterText = Entry->GetFilterText().Replace(TEXT(" "), TEXT(""));
    	bool bIsValid = true;
    	float MatchLength = 0.0f;
    	for (const FString& Token : FilterTockens) {
    		if (!FilterText.Contains(Token)) {
    			bIsValid = false;
    		}
    		MatchLength += Token.Len();
    	}
    	if (bIsValid) {
    		float MatchPercentage = MatchLength / ((float)FilterText.Len());
    		if (BestMatchPercentage < MatchPercentage) {
    			BestMatchPercentage = MatchPercentage;
    			BestMatch = Entry;
    		}
    	}
    	if (!bIsValid) {
    		Entry->Filter(SharedThis(this), OnFiltered);
    		bIsValid = Entry->GetChilds().Num() > 0;
    	} else {
    		CallFilterValid(Entry, OnFiltered);
    	}
    	if (bIsValid) Filtered.Add(Entry);
    	Entry->OnFiltered(bIsValid, this);
    	OnFiltered(this, Entry, bIsValid);
    }
    return Filtered;
}

void FFINScriptActionSelectionTextFilter::Reset() {
	BestMatch = nullptr;
	BestMatchPercentage = 0.0f;
}

FString FFINScriptActionSelectionTextFilter::GetFilterText() {
	FString FilterText;
	for (const FString& Token : FilterTockens) {
		if (FilterText.Len() > 0) FilterText = FilterText.Append(" ");
		FilterText.Append(Token);
	}
	return FilterText;
}

void FFINScriptActionSelectionTextFilter::SetFilterText(const FString& FilterText) {
	FString TokenList = FilterText;
	FilterTockens.Empty();
	while (TokenList.Len() > 0) {
		FString Token;
		if (TokenList.Split(" ", &Token, &TokenList)) {
			if (Token.Len() > 0) FilterTockens.Add(Token);
		} else {
			FilterTockens.Add(TokenList);
			TokenList = "";
		}
	}
}

TArray<TSharedPtr<FFINScriptActionSelectionEntry>> FFINScriptActionSelectionEntry::GetAllChilds() {
	if (bReloadCache) {
		bReloadCache = false;
		Cache = GenerateCache();
		ResetFilter();
	}
	return Cache;
}

void FFINScriptActionSelectionEntry::Expand(const TSharedPtr<STreeView<TSharedPtr<FFINScriptActionSelectionEntry>>>& View) {
	for (const TSharedPtr<FFINScriptActionSelectionEntry>& Entry : GetChilds()) {
		View->SetItemExpansion(Entry, true);
		Entry->Expand(View);
	}
}

void FFINScriptActionSelectionEntry::Filter(const TSharedPtr<FFINScriptActionSelectionFilter>& Filter, TFunction<void(FFINScriptActionSelectionFilter*, const TSharedPtr<FFINScriptActionSelectionEntry>&, bool)> OnFiltered) {
	FilteredCache = Filter->Filter(FilteredCache, OnFiltered);
}

void FFINScriptActionSelectionEntry::ResetFilter() {
	FilteredCache = GetAllChilds();
	for (const TSharedPtr<FFINScriptActionSelectionEntry>& Child : FilteredCache) {
		Child->ResetFilter();
	}
}

TSharedRef<SWidget> FFINScriptActionSelectionFuncAction::GetTreeWidget() {
	return SNew(SBorder)
	.BorderImage_Lambda([this]() {
		return bSelected ? &SelectedBrush : &UnselectedBrush;
	})
	.Content()[
		SNew(STextBlock)
		.Text_Lambda([this]() {
			return Func->GetDisplayName();
		})
		.HighlightText_Lambda([this](){ return FText::FromString(LastFilter); })
		.HighlightColor(FLinearColor(FColor::Yellow))
		.HighlightShape(&HighlightBrush)
	];
}

FString FFINScriptActionSelectionFuncAction::GetFilterText() const {
	return Func->GetDisplayName().ToString();
}

void FFINScriptActionSelectionFuncAction::OnFiltered(bool bFilterPassed, FFINScriptActionSelectionFilter* Filter) {
	FFINScriptActionSelectionTextFilter* TextFilter = dynamic_cast<FFINScriptActionSelectionTextFilter*>(Filter);
	if (TextFilter) LastFilter = TextFilter->GetFilterText();
}

void FFINScriptActionSelectionFuncAction::ResetFilter() {
	FFINScriptActionSelectionAction::ResetFilter();
	LastFilter = "";
}

void FFINScriptActionSelectionFuncAction::ExecuteAction() {
	UFINScriptReflectedFuncNode* Node = NewObject<UFINScriptReflectedFuncNode>();
	Node->Pos = Context.CreationLocation;
	Node->SetFunction(Func);
	for (UFINScriptPin* Pin : Node->GetNodePins()) {
		if (Context.Pin && Pin->CanConnect(Context.Pin)) {
			Pin->AddConnection(Context.Pin);
			break;
		}
	}
	Context.Graph->AddNode(Node);
}

TSharedRef<SWidget> FFINScriptActionSelectionTypeCategory::GetTreeWidget() {
	return SNew(STextBlock)
	.Text_Lambda([this]() {
		return Type->GetDisplayName();
	})
	.HighlightText_Lambda([this](){ return FText::FromString(LastFilter); })
    .HighlightColor(FLinearColor(FColor::Yellow))
    .HighlightShape(&HighlightBrush);
}

TArray<TSharedPtr<FFINScriptActionSelectionEntry>> FFINScriptActionSelectionTypeCategory::GenerateCache() {
	TArray<TSharedPtr<FFINScriptActionSelectionEntry>> Childs;
	TArray<UFINFunction*> Functions = Type->GetFunctions(false);
	for (UFINFunction* Function : Functions) {
		Childs.Add(MakeShared<FFINScriptActionSelectionFuncAction>(Function, Context));
	}
	return Childs;
}

FString FFINScriptActionSelectionTypeCategory::GetFilterText() const {
	return Type->GetDisplayName().ToString();
}

void FFINScriptActionSelectionTypeCategory::OnFiltered(bool bFilterPassed, FFINScriptActionSelectionFilter* Filter) {
	FFINScriptActionSelectionTextFilter* TextFilter = dynamic_cast<FFINScriptActionSelectionTextFilter*>(Filter);
	if (TextFilter) LastFilter = TextFilter->GetFilterText();
}

void FFINScriptActionSelectionTypeCategory::ResetFilter() {
	FFINScriptActionSelectionCategory::ResetFilter();
	LastFilter = "";
}

void SFINScriptActionSelection::Construct(const FArguments& InArgs) {
	bContextSensitive = InArgs._ContextSensetive.Get();
	OnActionExecuted = InArgs._OnActionExecuted;

	Children.Add(SNew(SBorder)
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
					if (TextFilter->BestMatch.IsValid()) SelectAction(FindNextAction(TextFilter->BestMatch));
				})
			]
			+SGridPanel::Slot(0, 3).ColumnSpan(2)[
				SNew(SBox)
				.MaxDesiredHeight(500)
				.WidthOverride(400)
				.Content()[
					SAssignNew(View, STreeView<TSharedPtr<FFINScriptActionSelectionEntry>>)
					.OnGenerateRow_Lambda([](TSharedPtr<FFINScriptActionSelectionEntry> Entry, const TSharedRef<STableViewBase>& Base) {
						return SNew(STableRow<TSharedPtr<FFINScriptActionSelectionEntry>>, Base).Content()[
							Entry->GetTreeWidget()
						];
					})
					.OnGetChildren_Lambda([](TSharedPtr<FFINScriptActionSelectionEntry> Entry, TArray<TSharedPtr<FFINScriptActionSelectionEntry>>& Childs) {
						Childs = Entry->GetChilds();
					})
					.TreeItemsSource(&Filtered)
					.OnMouseButtonClick_Lambda([this](TSharedPtr<FFINScriptActionSelectionEntry> Entry) {
						ExecuteEntry(Entry);
						Close();
					})
				]
			]
		]);

	SearchBox->SetOnKeyDownHandler(FOnKeyDown::CreateSP(this, &SFINScriptActionSelection::OnKeyDown));
}

void SFINScriptActionSelection::SetFocus() {
	FSlateApplication::Get().SetKeyboardFocus(SearchBox);
}

SFINScriptActionSelection::SFINScriptActionSelection() : Children(this) {
	TextFilter = MakeShared<FFINScriptActionSelectionTextFilter>(TEXT(""));
	Filters.Add(TextFilter);
}

FVector2D SFINScriptActionSelection::ComputeDesiredSize(float) const {
	return Children[0]->GetDesiredSize();
}

FChildren* SFINScriptActionSelection::GetChildren() {
	return &Children;
}

void SFINScriptActionSelection::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const {
	if (Children.Num() > 0) ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(Children[0], FVector2D(), Children[0]->GetDesiredSize(), 1));
}

FReply SFINScriptActionSelection::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == EKeys::Down) {
		SelectNext();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Up) {
		SelectPrevious();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Enter) {
		ExecuteEntry(SelectedAction);
		Close();
	}
	return SPanel::OnKeyDown(MyGeometry, InKeyEvent);
}

void SFINScriptActionSelection::SetSource(const TArray<TSharedPtr<FFINScriptActionSelectionEntry>>& NewSource) {
	Source = NewSource;
	ResetFilters();
}

void SFINScriptActionSelection::AddSource(const TSharedPtr<FFINScriptActionSelectionEntry>& Entry) {
	Source.Add(Entry);
	ResetFilters();
}

void SFINScriptActionSelection::ClearSource() {
	Source.Empty();
	ResetFilters();
}

void SFINScriptActionSelection::ResetFilters() {
	Filtered = Source;
	for (const TSharedPtr<FFINScriptActionSelectionEntry>& Entry : Filtered) {
		Entry->ResetFilter();
	}
	for (const TSharedPtr<FFINScriptActionSelectionFilter>& Filter : Filters) {
		Filter->Reset();
	}
}

void SFINScriptActionSelection::SetMenu(const TSharedPtr<IMenu>& inMenu) {
	Menu = inMenu;
}

void SFINScriptActionSelection::Filter() {
	FilteredActions.Empty();
	ResetFilters();
	for (const TSharedPtr<FFINScriptActionSelectionFilter>& Filter : Filters) {
		Filtered = Filter->Filter(Filtered, [this](FFINScriptActionSelectionFilter* Filter, const TSharedPtr<FFINScriptActionSelectionEntry>& Entry, bool bIsValid) {
			if (bIsValid && dynamic_cast<FFINScriptActionSelectionAction*>(Entry.Get())) FilteredActions.Add(StaticCastSharedPtr<FFINScriptActionSelectionAction>(Entry));
		});
	}
}

void SFINScriptActionSelection::ExpandAll() {
	for (const TSharedPtr<FFINScriptActionSelectionEntry>& Entry : Filtered) {
		View->SetItemExpansion(Entry, true);
	}
}

TSharedPtr<FFINScriptActionSelectionAction> FindNextChildAction(const TSharedPtr<FFINScriptActionSelectionEntry>& Entry) {
	for (const TSharedPtr<FFINScriptActionSelectionEntry>& Child : Entry->GetChilds()) {
		if (dynamic_cast<FFINScriptActionSelectionAction*>(Entry.Get())) return StaticCastSharedPtr<FFINScriptActionSelectionAction>(Entry);
	}
	for (const TSharedPtr<FFINScriptActionSelectionEntry>& Child : Entry->GetChilds()) {
		TSharedPtr<FFINScriptActionSelectionAction> FoundEntry = FindNextChildAction(Child);
		if (FoundEntry.IsValid()) return FoundEntry;
	}
	return nullptr;
}

TSharedPtr<FFINScriptActionSelectionAction> SFINScriptActionSelection::FindNextAction(const TSharedPtr<FFINScriptActionSelectionEntry>& Entry) {
	if (dynamic_cast<FFINScriptActionSelectionAction*>(Entry.Get())) return StaticCastSharedPtr<FFINScriptActionSelectionAction>(Entry);
	return FindNextChildAction(Entry);
}

void SFINScriptActionSelection::SelectAction(const TSharedPtr<FFINScriptActionSelectionAction>& Action) {
	if (SelectedAction.IsValid()) SelectedAction->bSelected = false;
	SelectedAction = Action;
	if (SelectedAction.IsValid()) {
		SelectedAction->bSelected = true;
		View->RequestScrollIntoView(SelectedAction);
	}
}

void SFINScriptActionSelection::SelectNext() {
	int SelectedIndex = FilteredActions.Find(SelectedAction);
	if (!SelectedAction.IsValid()) SelectedIndex = -1;
	++SelectedIndex;
	if (SelectedIndex >= FilteredActions.Num()) SelectedIndex = 0;
	if (FilteredActions.Num() > 0) SelectAction(FilteredActions[SelectedIndex]);
	else SelectAction(nullptr);
}

void SFINScriptActionSelection::SelectPrevious() {
	int SelectedIndex = FilteredActions.Find(SelectedAction);
	if (!SelectedAction.IsValid()) SelectedIndex = -1;
	--SelectedIndex;
	if (SelectedIndex < 0) SelectedIndex = FilteredActions.Num()-1;
	if (FilteredActions.Num() > 0) SelectAction(FilteredActions[SelectedIndex]);
	else SelectAction(nullptr);
}

void SFINScriptActionSelection::Close() {
	if (Menu) {
		FSlateApplication::Get().DismissMenu(Menu);
	}
}

void SFINScriptActionSelection::ExecuteEntry(const TSharedPtr<FFINScriptActionSelectionEntry>& Entry) {
	if (dynamic_cast<FFINScriptActionSelectionAction*>(Entry.Get())) {
		TSharedPtr<FFINScriptActionSelectionAction> Action = StaticCastSharedPtr<FFINScriptActionSelectionAction>(Entry);
		Action->ExecuteAction();
		OnActionExecuted.Execute(Action);
	}
}
