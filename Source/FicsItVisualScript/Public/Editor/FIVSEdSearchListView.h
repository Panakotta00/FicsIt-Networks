#pragma once

#include "CoreMinimal.h"
#include "FIVSEdTextSearch.h"
#include "SlateBasics.h"
#include "Widgets/Input/SSearchBox.h"

/**
 * This class allows you to have a filtered and searchable list view of a given type.
 */
template<typename T>
class SFIVSEdSearchListView : public SCompoundWidget {
public:
	struct FEntry {
		T Element;

		FEntry(const T& InElement) : Element(InElement) {}
	};
	
	DECLARE_DELEGATE_RetVal_OneParam(FString, FGetSearchableText, T&)
	DECLARE_DELEGATE_RetVal_OneParam(TSharedRef<SWidget>, FGetElementWidget, const T&)
	DECLARE_DELEGATE_OneParam(FSelectionChanged, T&)
	DECLARE_DELEGATE_OneParam(FCommited, T&)
	
private:
	SLATE_BEGIN_ARGS(SFIVSEdSearchListView<T>) : _BackgroundBrush(&DefaultBackgroundBrush) {}
	SLATE_EVENT(FGetSearchableText, OnGetSearchableText)
	SLATE_EVENT(FGetElementWidget, OnGetElementWidget)
	SLATE_EVENT(FSelectionChanged, OnSelectionChanged)
	SLATE_EVENT(FCommited, OnCommited)
	SLATE_ATTRIBUTE(const FSlateBrush*, BackgroundBrush)
	SLATE_END_ARGS()

	const static inline FSlateColorBrush DefaultBackgroundBrush = FSlateColorBrush(FColor::FromHex(TEXT("0F0F0F")));

	TArray<T> Elements;
	TArray<TSharedPtr<FEntry>> FilteredEntries;
	TAttribute<const FSlateBrush*> BackgroundBrush;

	FFIVSEdTextSearch Search;
	TSharedPtr<SListView<TSharedPtr<FEntry>>> ListView;
	TSharedPtr<SSearchBox> SearchBox;

public:
	FGetSearchableText OnGetSearchableText;
	FGetElementWidget OnGetElementWidget;
	FSelectionChanged OnSelectionChanged;
	FCommited OnCommited;

	void Construct(const FArguments& InArgs, TArray<T> InElements) {
		Elements = InElements;

		OnGetSearchableText = InArgs._OnGetSearchableText;
		OnGetElementWidget = InArgs._OnGetElementWidget;
		OnSelectionChanged = InArgs._OnSelectionChanged;
		OnCommited = InArgs._OnCommited;
		BackgroundBrush = InArgs._BackgroundBrush;

		ChildSlot[
			SNew(SBorder)
			.BorderImage(BackgroundBrush)
			.Content()[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()[
					SAssignNew(SearchBox, SSearchBox)
					.OnKeyDownHandler_Lambda([this](const FGeometry&, const FKeyEvent& KeyEvent) {
						if (KeyEvent.GetKey() == EKeys::Escape) {
							return FReply::Handled().ClearUserFocus();
						}
						if (KeyEvent.GetKey() == EKeys::Enter) {
							if (ListView->GetNumItemsSelected() > 0) {
								OnCommited.Execute(ListView->GetSelectedItems()[0]->Element);
								return FReply::Handled().ClearUserFocus();
							}
						}
						return FReply::Unhandled();
					})
					.SelectAllTextWhenFocused(true)
					.OnTextChanged_Lambda([this](FText InText) {
						Search.SetSearchText(InText.ToString());
						FilterEntries();
					})
				]
				+SVerticalBox::Slot()[
					SAssignNew(ListView, SListView<TSharedPtr<FEntry>>)
					.SelectionMode(ESelectionMode::Single)
					.ListItemsSource(&FilteredEntries)
					.OnMouseButtonClick_Lambda([this](TSharedPtr<FEntry> InEntry) {
						OnSelectionChanged.ExecuteIfBound(InEntry->Element);
						OnCommited.ExecuteIfBound(InEntry->Element);
					})
					.OnSelectionChanged_Lambda([this](TSharedPtr<FEntry> InEntry, ESelectInfo::Type) {
						if (InEntry) OnSelectionChanged.ExecuteIfBound(InEntry->Element);
					})
					.OnGenerateRow_Lambda([this](TSharedPtr<FEntry> InEntry, const TSharedRef<STableViewBase>& Row) {
						return SNew(STableRow<TSharedPtr<FEntry>>, Row)[
							OnGetElementWidget.Execute(InEntry->Element)
						];
					})
				]
			]
		];

		FilterEntries();

		RegisterActiveTimer(0.016f, FWidgetActiveTimerDelegate::CreateLambda([this](double InCurrentTime, float InDeltaTime) {
			FWidgetPath FocusMe;
			FSlateApplication::Get().GeneratePathToWidgetChecked( SearchBox.ToSharedRef(), FocusMe );
			FSlateApplication::Get().SetKeyboardFocus( FocusMe, EFocusCause::SetDirectly );
			return EActiveTimerReturnType::Stop;
		}));
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override {
		if (InKeyEvent.GetKey() == EKeys::Enter) {
			if (ListView->GetSelectedItems().Num() > 0) OnCommited.Execute(ListView->GetSelectedItems()[0]->Element);
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	void FilterEntries() {
		FilteredEntries.Empty();
		TSharedPtr<FEntry> MostRelevantEntry;
		int MostRelevantSignificance = TNumericLimits<int>::Min();
		for (T& Element : Elements) {
			FString SearchableText = OnGetSearchableText.Execute(Element);
			int Significance;
			if (Search.Search(SearchableText, &Significance)) {
				TSharedPtr<FEntry> Entry = MakeShared<FEntry>(Element);
				if (MostRelevantSignificance < Significance) {
					MostRelevantSignificance = Significance;
					MostRelevantEntry = Entry;
				}
				FilteredEntries.Add(Entry);
			}
		}

		ListView->RequestListRefresh();
		ListView->SetItemSelection(MostRelevantEntry, true);
	}
};

