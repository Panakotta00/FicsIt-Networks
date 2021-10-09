#pragma once

#include "SlateBasics.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSScriptNode.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSGraph.h"
#include "FicsItNetworks/Reflection/FINFunction.h"
#include "FicsItNetworks/Reflection/FINStruct.h"
#include "Widgets/Input/SSearchBox.h"

struct FFIVSEdActionSelectionAction;
DECLARE_DELEGATE_OneParam(FFINScriptActionSelectionOnActionExecuted, const TSharedPtr<FFIVSEdActionSelectionAction>&);

struct FFINScriptNodeCreationContext {
	UFIVSGraph* Graph;
	FVector2D CreationLocation;
	UFIVSPin* Pin;

	FFINScriptNodeCreationContext(UFIVSGraph* inGraph, const FVector2D& inCreationLocation, UFIVSPin* inPin) : Graph(inGraph), CreationLocation(inCreationLocation), Pin(inPin) {}
};

struct FFIVSEdActionSelectionEntry;

struct FFIVSEdActionSelectionFilter : TSharedFromThis<FFIVSEdActionSelectionFilter> {
	virtual ~FFIVSEdActionSelectionFilter() = default;
	
	/**
	 * Filteres the given action selection entry list,
	 * and returns the filtered list.
	 */
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Filter(const TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>& ToFilter, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)> OnEntryHandled = [](auto, auto, auto){}) = 0;

	/**
	 * Resets the filter cache
	 */
	virtual void Reset() {}
};

struct FFIVSEdActionSelectionTextFilter : FFIVSEdActionSelectionFilter {
private:
	TArray<FString> FilterTockens;

	void CallFilterValid(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)> OnFiltered);

public:
	TSharedPtr<FFIVSEdActionSelectionEntry> BestMatch;
	float BestMatchPercentage = 0.0f;

	FFIVSEdActionSelectionTextFilter(const FString& FilterText);
	
	// Begin FFINScriptActionSelectionFilter
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Filter(const TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>& ToFilter, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)>) override;
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

struct FFIVSEdActionSelectionEntry : TSharedFromThis<FFIVSEdActionSelectionEntry> {
protected:
	bool bReloadCache = true;
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Cache;
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> FilteredCache;

public:
	virtual ~FFIVSEdActionSelectionEntry() = default;

	/**
	 * Returns the filtered child list
	 */
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> GetChilds() { return FilteredCache; }

	/**
	 * Generates a list of all new children to be stored in the cache
	 */
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> GenerateCache() = 0;

	/**
	 * Returns all the childs in a list
	 */
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> GetAllChilds();

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
	virtual void Expand(const TSharedPtr<STreeView<TSharedPtr<FFIVSEdActionSelectionEntry>>>& View);

	/**
	 * Gets called by the filter if filtered
	 */
	virtual void OnFiltered(bool bFilterPassed, FFIVSEdActionSelectionFilter* Filter) {};

	/**
	 * Filters the Children with the given filter
	 */
	virtual void Filter(const TSharedPtr<FFIVSEdActionSelectionFilter>& Filter, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)> OnFiltered);

	/**
	 * Resets all appllied filters on the children
	 */
	virtual void ResetFilter();
};

struct FFIVSEdActionSelectionAction : FFIVSEdActionSelectionEntry {
	bool bSelected = false;

	FSlateColorBrush SelectedBrush = FSlateColorBrush(FLinearColor(0.3, 0.3, 0.3, 0.3));
	FSlateColorBrush UnselectedBrush = FSlateColorBrush(FColor::Transparent);
	FSlateColorBrush HighlightBrush = FSlateColorBrush(FColor::Orange);

	/**
	 * Executes this action
	 */
	virtual void ExecuteAction() = 0;
};

struct FFIVSEdActionSelectionFuncAction : FFIVSEdActionSelectionAction {
private:
	UFINFunction* Func = nullptr;
	FString LastFilter = "";

	FFINScriptNodeCreationContext Context;

public:
	FFIVSEdActionSelectionFuncAction(UFINFunction* Func, const FFINScriptNodeCreationContext& Context) : Func(Func), Context(Context) {}

	// Begin FFINScriptActionSelectionEntry
	virtual TSharedRef<SWidget> GetTreeWidget() override;
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> GenerateCache() override { return TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>(); }
	virtual FString GetFilterText() const override;
	virtual void OnFiltered(bool bFilterPassed, FFIVSEdActionSelectionFilter* Filter) override;
	virtual void ResetFilter() override;
	// End FFINScriptActionSelectionEntry

	// Begin FFINScriptActionSelectionAction
	virtual void ExecuteAction() override;
	// End FFINScriptActionSelectionAction
};

DECLARE_DELEGATE_OneParam(FFIVSEdActionSelectionGenericActionInit, UFIVSScriptNode*)

struct FFIVSEdActionSelectionGenericAction : FFIVSEdActionSelectionAction {
private:
	TSubclassOf<UFIVSScriptNode> ScriptNode = nullptr;
	FString LastFilter = "";

	FFINScriptNodeCreationContext Context;


public:
	FFIVSEdActionSelectionGenericActionInit Init;
	
	FFIVSEdActionSelectionGenericAction(TSubclassOf<UFIVSScriptNode> ScriptNode, const FFINScriptNodeCreationContext& Context) : ScriptNode(ScriptNode), Context(Context) {}

	// Begin FFINScriptActionSelectionEntry
	virtual TSharedRef<SWidget> GetTreeWidget() override;
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> GenerateCache() override { return TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>(); }
	virtual FString GetFilterText() const override;
	virtual void OnFiltered(bool bFilterPassed, FFIVSEdActionSelectionFilter* Filter) override;
	virtual void ResetFilter() override;
	// End FFINScriptActionSelectionEntry

	// Begin FFINScriptActionSelectionAction
	virtual void ExecuteAction() override;
	// End FFINScriptActionSelectionAction
};

struct FFIVSEdActionSelectionCategory : FFIVSEdActionSelectionEntry {
	FSlateColorBrush HighlightBrush = FSlateColorBrush(FColor::Orange);
};

struct FFIVSEdActionSelectionTypeCategory : FFIVSEdActionSelectionCategory {
private:
	UFINStruct* Type;
	FString LastFilter = "";

	FFINScriptNodeCreationContext Context;

public:
	FFIVSEdActionSelectionTypeCategory(UFINStruct* Type, const FFINScriptNodeCreationContext& Context) : Type(Type), Context(Context) {}

	// Begin FFINScriptNodeSelectionEntry
	virtual TSharedRef<SWidget> GetTreeWidget() override;
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> GenerateCache() override;
	virtual FString GetFilterText() const override;
	virtual void OnFiltered(bool bFilterPassed, FFIVSEdActionSelectionFilter* Filter) override;
	virtual void ResetFilter() override;
	// End FFINScriptNodeSelectionEntry
};

class SFIVSEdActionSelection : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SFIVSEdActionSelection) {}
		SLATE_ATTRIBUTE(bool, ContextSensetive)

		SLATE_EVENT(FFINScriptActionSelectionOnActionExecuted, OnActionExecuted)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);
	void SetFocus();

private:
	bool bContextSensitive = false;
	FFINScriptActionSelectionOnActionExecuted OnActionExecuted;
	
	TSharedPtr<STreeView<TSharedPtr<FFIVSEdActionSelectionEntry>>> View;
	TSharedPtr<SSearchBox> SearchBox;
	
	TSharedPtr<IMenu> Menu;
	TSharedPtr<FFIVSEdActionSelectionAction> SelectedAction;

	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Source;
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Filtered;

	TArray<TSharedPtr<FFIVSEdActionSelectionFilter>> Filters;
	TSharedPtr<FFIVSEdActionSelectionTextFilter> TextFilter;

	TArray<TSharedPtr<FFIVSEdActionSelectionAction>> FilteredActions;

	FSlateColorBrush BackgroundBrush = FSlateColorBrush(FLinearColor(0.02, 0.02, 0.02));
public:
	SFIVSEdActionSelection();
	
	// Begin SPanel
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// End SPanel

	/**
	 * Sets the used source
	 */
	void SetSource(const TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>& NewSource);

	/**
	 * Adds the given entry to the source
	 */
	void AddSource(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry);

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
	TSharedPtr<FFIVSEdActionSelectionAction> FindNextAction(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry);
	
	/**
	 * Selects the given action
	 */
	void SelectAction(const TSharedPtr<FFIVSEdActionSelectionAction>& Action);

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
	void ExecuteEntry(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry);
};
