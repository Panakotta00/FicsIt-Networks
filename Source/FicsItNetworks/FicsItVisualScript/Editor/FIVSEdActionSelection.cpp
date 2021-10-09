#include "FIVSEdActionSelection.h"

void FFIVSEdActionSelectionTextFilter::CallFilterValid(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entries, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)> OnFiltered) {
	OnFiltered(this, Entries, true);
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : Entries->GetChilds()) {
		CallFilterValid(Entry, OnFiltered);
	}
}

FFIVSEdActionSelectionTextFilter::FFIVSEdActionSelectionTextFilter(const FString& Filter) {
    SetFilterText(Filter);
}

TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> FFIVSEdActionSelectionTextFilter::Filter(const TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>& ToFilter, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)> OnFiltered) {
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Filtered;
    for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : ToFilter) {
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

void FFIVSEdActionSelectionTextFilter::Reset() {
	BestMatch = nullptr;
	BestMatchPercentage = 0.0f;
}

FString FFIVSEdActionSelectionTextFilter::GetFilterText() {
	FString FilterText;
	for (const FString& Token : FilterTockens) {
		if (FilterText.Len() > 0) FilterText = FilterText.Append(" ");
		FilterText.Append(Token);
	}
	return FilterText;
}

void FFIVSEdActionSelectionTextFilter::SetFilterText(const FString& FilterText) {
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

TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> FFIVSEdActionSelectionEntry::GetAllChilds() {
	if (bReloadCache) {
		bReloadCache = false;
		Cache = GenerateCache();
		ResetFilter();
	}
	return Cache;
}

void FFIVSEdActionSelectionEntry::Expand(const TSharedPtr<STreeView<TSharedPtr<FFIVSEdActionSelectionEntry>>>& View) {
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : GetChilds()) {
		View->SetItemExpansion(Entry, true);
		Entry->Expand(View);
	}
}

void FFIVSEdActionSelectionEntry::Filter(const TSharedPtr<FFIVSEdActionSelectionFilter>& Filter, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)> OnFiltered) {
	FilteredCache = Filter->Filter(FilteredCache, OnFiltered);
}

void FFIVSEdActionSelectionEntry::ResetFilter() {
	FilteredCache = GetAllChilds();
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Child : FilteredCache) {
		Child->ResetFilter();
	}
}

TSharedRef<SWidget> FFIVSEdActionSelectionFuncAction::GetTreeWidget() {
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

FString FFIVSEdActionSelectionFuncAction::GetFilterText() const {
	return Func->GetDisplayName().ToString();
}

void FFIVSEdActionSelectionFuncAction::OnFiltered(bool bFilterPassed, FFIVSEdActionSelectionFilter* Filter) {
	FFIVSEdActionSelectionTextFilter* TextFilter = dynamic_cast<FFIVSEdActionSelectionTextFilter*>(Filter);
	if (TextFilter) LastFilter = TextFilter->GetFilterText();
}

void FFIVSEdActionSelectionFuncAction::ResetFilter() {
	FFIVSEdActionSelectionAction::ResetFilter();
	LastFilter = "";
}

void FFIVSEdActionSelectionFuncAction::ExecuteAction() {
	UFIVSNodeCallFunction* Node = NewObject<UFIVSNodeCallFunction>(Context.Graph);
	Node->Pos = Context.CreationLocation;
	Node->SetFunction(Func);
	Node->InitPins();
	for (UFIVSPin* Pin : Node->GetNodePins()) {
		if (Context.Pin && Pin->CanConnect(Context.Pin)) {
			Pin->AddConnection(Context.Pin);
			break;
		}
	}
	Context.Graph->AddNode(Node);
}


TSharedRef<SWidget> FFIVSEdActionSelectionGenericAction::GetTreeWidget() {
	return SNew(SBorder)
	.BorderImage_Lambda([this]() {
		return bSelected ? &SelectedBrush : &UnselectedBrush;
	})
	.Content()[
		SNew(STextBlock)
		.Text_Lambda([this]() {
			return FText::FromString(GetFilterText());
		})
		.HighlightText_Lambda([this](){ return FText::FromString(LastFilter); })
		.HighlightColor(FLinearColor(FColor::Yellow))
		.HighlightShape(&HighlightBrush)
	];
}

FString FFIVSEdActionSelectionGenericAction::GetFilterText() const {
	UFIVSScriptNode* Node = const_cast<UFIVSScriptNode*>(GetDefault<UFIVSScriptNode>(ScriptNode));
	Init.ExecuteIfBound(Node);
	return Node->GetNodeName();
}

void FFIVSEdActionSelectionGenericAction::OnFiltered(bool bFilterPassed, FFIVSEdActionSelectionFilter* Filter) {
	FFIVSEdActionSelectionTextFilter* TextFilter = dynamic_cast<FFIVSEdActionSelectionTextFilter*>(Filter);
	if (TextFilter) LastFilter = TextFilter->GetFilterText();
}

void FFIVSEdActionSelectionGenericAction::ResetFilter() {
	FFIVSEdActionSelectionAction::ResetFilter();
	LastFilter = "";
}

void FFIVSEdActionSelectionGenericAction::ExecuteAction() {
	UFIVSScriptNode* Node = NewObject<UFIVSScriptNode>(Context.Graph, ScriptNode);
	Node->Pos = Context.CreationLocation;
	Init.ExecuteIfBound(Node);
	Node->InitPins();
	for (UFIVSPin* Pin : Node->GetNodePins()) {
		if (Context.Pin && Pin->CanConnect(Context.Pin)) {
			Pin->AddConnection(Context.Pin);
			break;
		}
	}
	Context.Graph->AddNode(Node);
}

TSharedRef<SWidget> FFIVSEdActionSelectionTypeCategory::GetTreeWidget() {
	return SNew(STextBlock)
	.Text_Lambda([this]() {
		return Type->GetDisplayName();
	})
	.HighlightText_Lambda([this](){ return FText::FromString(LastFilter); })
    .HighlightColor(FLinearColor(FColor::Yellow))
    .HighlightShape(&HighlightBrush);
}

TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> FFIVSEdActionSelectionTypeCategory::GenerateCache() {
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Childs;
	TArray<UFINFunction*> Functions = Type->GetFunctions(false);
	for (UFINFunction* Function : Functions) {
		Childs.Add(MakeShared<FFIVSEdActionSelectionFuncAction>(Function, Context));
	}
	TArray<UFINProperty*> Properties = Type->GetProperties(false);
	for (UFINProperty* Property : Properties) {
		EFINRepPropertyFlags Flags = Property->GetPropertyFlags();
		if (Flags & FIN_Prop_Attrib) {
			TSharedRef<FFIVSEdActionSelectionGenericAction> GetAction = MakeShared<FFIVSEdActionSelectionGenericAction>(UFIVSNodeGetProperty::StaticClass(), Context);
			GetAction->Init.BindLambda([Property](UFIVSNode* Node) {
				UFIVSNodeGetProperty* GetNode = Cast<UFIVSNodeGetProperty>(Node);
				GetNode->SetProperty(Property);
			});
			Childs.Add(GetAction);
			if (!(Flags & FIN_Prop_ReadOnly)) {
				TSharedRef<FFIVSEdActionSelectionGenericAction> SetAction = MakeShared<FFIVSEdActionSelectionGenericAction>(UFIVSNodeSetProperty::StaticClass(), Context);
				SetAction->Init.BindLambda([Property](UFIVSNode* Node) {
					UFIVSNodeSetProperty* SetNode = Cast<UFIVSNodeSetProperty>(Node);
					SetNode->SetProperty(Property);
				});
				Childs.Add(SetAction);
			}
		}
	}
	return Childs;
}

FString FFIVSEdActionSelectionTypeCategory::GetFilterText() const {
	return Type->GetDisplayName().ToString();
}

void FFIVSEdActionSelectionTypeCategory::OnFiltered(bool bFilterPassed, FFIVSEdActionSelectionFilter* Filter) {
	FFIVSEdActionSelectionTextFilter* TextFilter = dynamic_cast<FFIVSEdActionSelectionTextFilter*>(Filter);
	if (TextFilter) LastFilter = TextFilter->GetFilterText();
}

void FFIVSEdActionSelectionTypeCategory::ResetFilter() {
	FFIVSEdActionSelectionCategory::ResetFilter();
	LastFilter = "";
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
					if (TextFilter->BestMatch.IsValid()) SelectAction(FindNextAction(TextFilter->BestMatch));
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
					.OnGetChildren_Lambda([](TSharedPtr<FFIVSEdActionSelectionEntry> Entry, TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>& Childs) {
						Childs = Entry->GetChilds();
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
		ExecuteEntry(SelectedAction);
		Close();
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
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : Filtered) {
		Entry->ResetFilter();
	}
	for (const TSharedPtr<FFIVSEdActionSelectionFilter>& Filter : Filters) {
		Filter->Reset();
	}
}

void SFIVSEdActionSelection::SetMenu(const TSharedPtr<IMenu>& inMenu) {
	Menu = inMenu;
}

void SFIVSEdActionSelection::Filter() {
	FilteredActions.Empty();
	ResetFilters();
	for (const TSharedPtr<FFIVSEdActionSelectionFilter>& Filter : Filters) {
		Filtered = Filter->Filter(Filtered, [this](FFIVSEdActionSelectionFilter* Filter, const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry, bool bIsValid) {
			if (bIsValid && dynamic_cast<FFIVSEdActionSelectionAction*>(Entry.Get())) FilteredActions.Add(StaticCastSharedPtr<FFIVSEdActionSelectionAction>(Entry));
		});
	}
}

void SFIVSEdActionSelection::ExpandAll() {
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry : Filtered) {
		View->SetItemExpansion(Entry, true);
	}
}

TSharedPtr<FFIVSEdActionSelectionAction> FindNextChildAction(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry) {
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Child : Entry->GetChilds()) {
		if (dynamic_cast<FFIVSEdActionSelectionAction*>(Entry.Get())) return StaticCastSharedPtr<FFIVSEdActionSelectionAction>(Entry);
	}
	for (const TSharedPtr<FFIVSEdActionSelectionEntry>& Child : Entry->GetChilds()) {
		TSharedPtr<FFIVSEdActionSelectionAction> FoundEntry = FindNextChildAction(Child);
		if (FoundEntry.IsValid()) return FoundEntry;
	}
	return nullptr;
}

TSharedPtr<FFIVSEdActionSelectionAction> SFIVSEdActionSelection::FindNextAction(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry) {
	if (dynamic_cast<FFIVSEdActionSelectionAction*>(Entry.Get())) return StaticCastSharedPtr<FFIVSEdActionSelectionAction>(Entry);
	return FindNextChildAction(Entry);
}

void SFIVSEdActionSelection::SelectAction(const TSharedPtr<FFIVSEdActionSelectionAction>& Action) {
	if (SelectedAction.IsValid()) SelectedAction->bSelected = false;
	SelectedAction = Action;
	if (SelectedAction.IsValid()) {
		SelectedAction->bSelected = true;
		View->RequestScrollIntoView(SelectedAction);
	}
}

void SFIVSEdActionSelection::SelectNext() {
	int SelectedIndex = FilteredActions.Find(SelectedAction);
	if (!SelectedAction.IsValid()) SelectedIndex = -1;
	++SelectedIndex;
	if (SelectedIndex >= FilteredActions.Num()) SelectedIndex = 0;
	if (FilteredActions.Num() > 0) SelectAction(FilteredActions[SelectedIndex]);
	else SelectAction(nullptr);
}

void SFIVSEdActionSelection::SelectPrevious() {
	int SelectedIndex = FilteredActions.Find(SelectedAction);
	if (!SelectedAction.IsValid()) SelectedIndex = -1;
	--SelectedIndex;
	if (SelectedIndex < 0) SelectedIndex = FilteredActions.Num()-1;
	if (FilteredActions.Num() > 0) SelectAction(FilteredActions[SelectedIndex]);
	else SelectAction(nullptr);
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
