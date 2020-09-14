#pragma once

#include "FINScriptGraph.h"
#include "SlateBasics.h"
#include "SSearchBox.h"
#include "Network/FINTypeManager.h"

struct FFINScriptActionSelectionAction;
DECLARE_DELEGATE_OneParam(FFINScriptActionSelectionOnActionExecuted, const TSharedPtr<FFINScriptActionSelectionAction>&);

struct FFINScriptNodeCreationContext {
	UFINScriptGraph* Graph;
	FVector2D CreationLocation;
	TSharedPtr<FFINScriptPin> Pin;

	FFINScriptNodeCreationContext(UFINScriptGraph* inGraph, const FVector2D& inCreationLocation, const TSharedPtr<FFINScriptPin>& inPin) : Graph(inGraph), CreationLocation(inCreationLocation), Pin(inPin) {}
};

struct FFINScriptActionSelectionEntry;

struct FFINScriptActionSelectionFilter : TSharedFromThis<FFINScriptActionSelectionFilter> {
	virtual ~FFINScriptActionSelectionFilter() = default;
	
	/**
	 * Filteres the given action selection entry list,
	 * and returns the filtered list.
	 */
	virtual TArray<TSharedPtr<FFINScriptActionSelectionEntry>> Filter(const TArray<TSharedPtr<FFINScriptActionSelectionEntry>>& ToFilter, TFunction<void(FFINScriptActionSelectionFilter*, const TSharedPtr<FFINScriptActionSelectionEntry>&, bool)> OnEntryHandled = [](auto, auto, auto){}) = 0;

	/**
	 * Resets the filter cache
	 */
	virtual void Reset() {}
};

struct FFINScriptActionSelectionTextFilter : FFINScriptActionSelectionFilter {
private:
	TArray<FString> FilterTockens;

	void CallFilterValid(const TSharedPtr<FFINScriptActionSelectionEntry>& Entry, TFunction<void(FFINScriptActionSelectionFilter*, const TSharedPtr<FFINScriptActionSelectionEntry>&, bool)> OnFiltered);

public:
	TSharedPtr<FFINScriptActionSelectionEntry> BestMatch;
	float BestMatchPercentage = 0.0f;

	FFINScriptActionSelectionTextFilter(const FString& FilterText);
	
	// Begin FFINScriptActionSelectionFilter
	virtual TArray<TSharedPtr<FFINScriptActionSelectionEntry>> Filter(const TArray<TSharedPtr<FFINScriptActionSelectionEntry>>& ToFilter, TFunction<void(FFINScriptActionSelectionFilter*, const TSharedPtr<FFINScriptActionSelectionEntry>&, bool)>) override;
	virtual void Reset() override;
	// End FFINScriptActionSelectionFilter

	/**
	 * Returns the filter text
	 */
	FString GetFilterText();

	/**
	 * Generates a new token list of the given string
	 */
	void SetFilterText(const FString& FilterText);
};

struct FFINScriptActionSelectionEntry : TSharedFromThis<FFINScriptActionSelectionEntry> {
protected:
	bool bReloadCache = true;
	TArray<TSharedPtr<FFINScriptActionSelectionEntry>> Cache;
	TArray<TSharedPtr<FFINScriptActionSelectionEntry>> FilteredCache;

public:
	virtual ~FFINScriptActionSelectionEntry() = default;

	/**
	 * Returns the filtered child list
	 */
	virtual TArray<TSharedPtr<FFINScriptActionSelectionEntry>> GetChilds() { return FilteredCache; }

	/**
	 * Generates a list of all new children to be stored in the cache
	 */
	virtual TArray<TSharedPtr<FFINScriptActionSelectionEntry>> GenerateCache() = 0;

	/**
	 * Returns all the childs in a list
	 */
	virtual TArray<TSharedPtr<FFINScriptActionSelectionEntry>> GetAllChilds();

	/**
	 * Constructs a widget that represents this action selection entry in the tree view
	 */
	virtual TSharedRef<SWidget> GetTreeWidget() = 0;
	
	/**
	 * Returns the text that is used for the generic filtering
	 */
	virtual FString GetFilterText() const = 0;
	
	/**
	 * Expands this and all child entries in the given tree view
	 */
	virtual void Expand(const TSharedPtr<STreeView<TSharedPtr<FFINScriptActionSelectionEntry>>>& View);

	/**
	 * Gets called by the filter if filtered
	 */
	virtual void OnFiltered(bool bFilterPassed, FFINScriptActionSelectionFilter* Filter) {};

	/**
	 * Filters the Children with the given filter
	 */
	virtual void Filter(const TSharedPtr<FFINScriptActionSelectionFilter>& Filter, TFunction<void(FFINScriptActionSelectionFilter*, const TSharedPtr<FFINScriptActionSelectionEntry>&, bool)> OnFiltered);

	/**
	 * Resets all appllied filters on the children
	 */
	virtual void ResetFilter();
};

struct FFINScriptActionSelectionAction : FFINScriptActionSelectionEntry {
	bool bSelected = false;

	FSlateColorBrush SelectedBrush = FSlateColorBrush(FLinearColor(0.3, 0.3, 0.3, 0.3));
	FSlateColorBrush UnselectedBrush = FSlateColorBrush(FColor::Transparent);
	FSlateColorBrush HighlightBrush = FSlateColorBrush(FColor::Orange);

	/**
	 * Executes this action
	 */
	virtual void ExecuteAction() = 0;
};

struct FFINScriptActionSelectionFuncAction : FFINScriptActionSelectionAction {
private:
	TSharedPtr<FFINFunction> Func;
	FString LastFilter = "";

	FFINScriptNodeCreationContext Context;

public:
	FFINScriptActionSelectionFuncAction(const TSharedPtr<FFINFunction>& Func, const FFINScriptNodeCreationContext& Context) : Func(Func), Context(Context) {}

	// Begin FFINScriptActionSelectionEntry
	virtual TSharedRef<SWidget> GetTreeWidget() override;
	virtual TArray<TSharedPtr<FFINScriptActionSelectionEntry>> GenerateCache() override { return TArray<TSharedPtr<FFINScriptActionSelectionEntry>>(); }
	virtual FString GetFilterText() const override;
	virtual void OnFiltered(bool bFilterPassed, FFINScriptActionSelectionFilter* Filter) override;
	virtual void ResetFilter() override;
	// End FFINScriptActionSelectionEntry

	// Begin FFINScriptActionSelectionAction
	virtual void ExecuteAction() override;
	// End FFINScriptActionSelectionAction
};

struct FFINScriptActionSelectionCategory : FFINScriptActionSelectionEntry {
	FSlateColorBrush HighlightBrush = FSlateColorBrush(FColor::Orange);
};

struct FFINScriptActionSelectionTypeCategory : FFINScriptActionSelectionCategory {
private:
	TSharedPtr<FFINType> Type;
	FString LastFilter = "";

	FFINScriptNodeCreationContext Context;

public:
	FFINScriptActionSelectionTypeCategory(const TSharedPtr<FFINType>& Type, const FFINScriptNodeCreationContext& Context) : Type(Type), Context(Context) {}

	// Begin FFINScriptNodeSelectionEntry
	virtual TSharedRef<SWidget> GetTreeWidget() override;
	virtual TArray<TSharedPtr<FFINScriptActionSelectionEntry>> GenerateCache() override;
	virtual FString GetFilterText() const override;
	virtual void OnFiltered(bool bFilterPassed, FFINScriptActionSelectionFilter* Filter) override;
	virtual void ResetFilter() override;
	// End FFINScriptNodeSelectionEntry
};

class SFINScriptActionSelection : public SPanel {
public:
	SLATE_BEGIN_ARGS(SFINScriptActionSelection) {}
		SLATE_ATTRIBUTE(bool, ContextSensetive)

		SLATE_EVENT(FFINScriptActionSelectionOnActionExecuted, OnActionExecuted)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	void SetFocus();

private:
	bool bContextSensitive = false;
	FFINScriptActionSelectionOnActionExecuted OnActionExecuted;
	
	TSlotlessChildren<SWidget> Children;
	TSharedPtr<STreeView<TSharedPtr<FFINScriptActionSelectionEntry>>> View;
	TSharedPtr<SSearchBox> SearchBox;
	
	TSharedPtr<IMenu> Menu;
	TSharedPtr<FFINScriptActionSelectionAction> SelectedAction;

	TArray<TSharedPtr<FFINScriptActionSelectionEntry>> Source;
	TArray<TSharedPtr<FFINScriptActionSelectionEntry>> Filtered;

	TArray<TSharedPtr<FFINScriptActionSelectionFilter>> Filters;
	TSharedPtr<FFINScriptActionSelectionTextFilter> TextFilter;

	TArray<TSharedPtr<FFINScriptActionSelectionAction>> FilteredActions;

	FSlateColorBrush BackgroundBrush = FSlateColorBrush(FLinearColor(0.02, 0.02, 0.02));
public:
	SFINScriptActionSelection();
	
	// Begin SPanel
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual FChildren* GetChildren() override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// End SPanel

	/**
	 * Sets the used source
	 */
	void SetSource(const TArray<TSharedPtr<FFINScriptActionSelectionEntry>>& NewSource);

	/**
	 * Adds the given entry to the source
	 */
	void AddSource(const TSharedPtr<FFINScriptActionSelectionEntry>& Entry);

	/**
	 * Cleares the used source
	 */
	void ClearSource();

	/**
	 * Resets all filter settings and also for childs
	 */
	void ResetFilters();

	/**
	 * Sets the menu context of this widget, if it is used in menue.
	 * used to close the menu when a action got executed or selection got canceled.
	 */
	void SetMenu(const TSharedPtr<IMenu>& inMenu);

	/**
	 * Updates the filtered node list
	 */
	void Filter();

	/**
	 * Expands all filtered entries
	 */
	void ExpandAll();

	/**
	 * Returns the next action next to the given entry or the entry it self if it is an action.
	 * nullptr if no action is found.
	 */
	TSharedPtr<FFINScriptActionSelectionAction> FindNextAction(const TSharedPtr<FFINScriptActionSelectionEntry>& Entry);
	
	/**
	 * Selects the given action
	 */
	void SelectAction(const TSharedPtr<FFINScriptActionSelectionAction>& Action);

	/**
	 * Selects the next action in the tress
	 */
	void SelectNext();

	/**
	 * Selectes the previous action in the tree
	 */
	void SelectPrevious();

	/**
	 * Closes this action selection
	 */
	void Close();

	/**
	 * Calls the given Entry, like executing the action
	 */
	void ExecuteEntry(const TSharedPtr<FFINScriptActionSelectionEntry>& Entry);
};
