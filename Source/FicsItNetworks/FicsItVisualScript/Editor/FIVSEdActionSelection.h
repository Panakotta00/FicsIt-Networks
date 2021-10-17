#pragma once

#include "SlateBasics.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSScriptNode.h"
#include "FicsItNetworks/FicsItVisualScript/Script/FIVSGraph.h"
#include "FicsItNetworks/Reflection/FINFunction.h"
#include "FicsItNetworks/Reflection/FINStruct.h"
#include "Framework/Text/SyntaxHighlighterTextLayoutMarshaller.h"
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
	 * Enables or disables the given entry if the filter applies or not.
	 * bForce is true if parent entry returned true from filter, this may be ignored if a filter has a higher priority, like context filter
	 * Returns true if this entry passes filter directly and causes all children to also pass.
	 */
	virtual bool Filter(TSharedPtr<FFIVSEdActionSelectionEntry> ToFilter, bool bForce) = 0;

	virtual void OnFiltered(TSharedPtr<FFIVSEdActionSelectionEntry> Entry, int Pass) {}

	virtual void Reset() {}
};

struct FFIVSEdActionSelectionTextFilter : FFIVSEdActionSelectionFilter {
private:
	TArray<FString> FilterTokens;
	
	void CallFilterValid(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry, TFunction<void(FFIVSEdActionSelectionFilter*, const TSharedPtr<FFIVSEdActionSelectionEntry>&, bool)> OnFiltered);
	
public:
	TMap<float, TSharedPtr<FFIVSEdActionSelectionEntry>> BestMatch;
	
	FFIVSEdActionSelectionTextFilter(const FString& FilterText);
	
	// Begin FFINScriptActionSelectionFilter
	virtual bool Filter(TSharedPtr<FFIVSEdActionSelectionEntry> ToFilter, bool bForce) override;
	virtual void OnFiltered(TSharedPtr<FFIVSEdActionSelectionEntry> Entry, int Pass) override;
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

struct FFIVSEdActionSelectionPinFilter : FFIVSEdActionSelectionFilter {
private:
	FFIVSFullPinType FilterPinType;

public:
	FFIVSEdActionSelectionPinFilter(FFIVSFullPinType InFilterPinType) : FilterPinType(InFilterPinType) {}

	// Begin FFIVSEdActionSelectionFilter
	virtual bool Filter(TSharedPtr<FFIVSEdActionSelectionEntry> ToFilter, bool bForce) override;
	virtual void Reset() override {
		//SetFilterPin(FFIVSFullPinType(FIVS_PIN_DATA_INPUT | FIVS_PIN_EXEC_OUTPUT));
	}
	// End FFIVSEdActionSelectionFilter

	void SetFilterPin(const FFIVSFullPinType& InFilterPin) {
		FilterPinType = InFilterPin;
	}
};

struct FFIVSEdActionSelectionEntry : TSharedFromThis<FFIVSEdActionSelectionEntry> {
public:
	virtual ~FFIVSEdActionSelectionEntry() = default;

	/**
	 * Defines if this action is enabled and should be shown in the action selection menu.
	 * This may be changed by a filter.
	 */
	bool bIsEnabled = true;

	/**
	 * Defines the highlight text of this action.
	 * This my be changed by a filter.
	 */
	FText HighlightText;
	
	/**
	 * Returns the child list
	 */
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> GetChildren() {
		return TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>();
	}

	/**
	 * returns the pins of the node action necessary for the context filter
	 */
	virtual FFIVSNodeAction GetSignature() { return FFIVSNodeAction(); }
	
	/**
	 * Constructs a widget that represents this action selection entry in the tree view
	 */
	virtual TSharedRef<SWidget> GetTreeWidget() = 0;

	/**
	 * Returns true if this action should be selectable (f.e. arrow navigation)
	 */
	virtual bool IsRelevant() const { return false; }
};

struct FFIVSEdActionSelectionAction : FFIVSEdActionSelectionEntry {
	/**
	 * Executes this action
	 */
	virtual void ExecuteAction() = 0;
};

DECLARE_DELEGATE_OneParam(FFIVSEdActionSelectionGenericActionInit, UFIVSScriptNode*)

struct FFIVSEdActionSelectionNodeAction : FFIVSEdActionSelectionAction {
private:
	FFIVSNodeAction NodeAction;
	
	FFINScriptNodeCreationContext Context;

	FSlateColorBrush HighlightBrush = FSlateColorBrush(FColor::Orange);

public:
	FFIVSEdActionSelectionNodeAction(const FFIVSNodeAction& NodeAction, const FFINScriptNodeCreationContext& Context) : NodeAction(NodeAction), Context(Context) {}
	
	// Begin FFINScriptActionSelectionEntry
	virtual TSharedRef<SWidget> GetTreeWidget() override;
	virtual FFIVSNodeAction GetSignature() override { return NodeAction; }
	virtual bool IsRelevant() const override { return true; }
	// End FFINScriptActionSelectionEntry

	// Begin FFINScriptActionSelectionAction
	virtual void ExecuteAction() override;
	// End FFINScriptActionSelectionAction
};

struct FFIVSEdActionSelectionCategory : FFIVSEdActionSelectionEntry {
	FSlateColorBrush HighlightBrush = FSlateColorBrush(FColor::Orange);
	
	FText Name;
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Children;

	FFIVSEdActionSelectionCategory(FText Name, TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Children) : Name(Name), Children(Children) {}
	
	// Begin FFINScriptNodeSelectionEntry
	virtual TSharedRef<SWidget> GetTreeWidget() override;
	virtual FFIVSNodeAction GetSignature() { return FFIVSNodeAction{nullptr, Name, FText::GetEmpty(), Name}; }
	virtual TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> GetChildren() override { return Children; }
	// End FFINScriptNodeSelectionEntry
};

class SFIVSEdActionSelection : public SCompoundWidget {
public:
	SLATE_BEGIN_ARGS(SFIVSEdActionSelection) {}
		SLATE_ATTRIBUTE(bool, ContextSensetive)

		SLATE_EVENT(FFINScriptActionSelectionOnActionExecuted, OnActionExecuted)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs, const FFIVSFullPinType& ContextPin = FFIVSFullPinType(FIVS_PIN_DATA_INPUT | FIVS_PIN_EXEC_OUTPUT));
	void SetFocus();

private:
	bool bContextSensitive = false;
	FFINScriptActionSelectionOnActionExecuted OnActionExecuted;
	
	TSharedPtr<STreeView<TSharedPtr<FFIVSEdActionSelectionEntry>>> View;
	TSharedPtr<SSearchBox> SearchBox;
	
	TSharedPtr<IMenu> Menu;
	
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Source;
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> Filtered;
	TMap<TSharedPtr<FFIVSEdActionSelectionEntry>, TArray<TSharedPtr<FFIVSEdActionSelectionEntry>>> FilteredChildren;
	TArray<TSharedPtr<FFIVSEdActionSelectionEntry>> AllRelevant;

	TArray<TSharedPtr<FFIVSEdActionSelectionFilter>> Filters;
	TSharedPtr<FFIVSEdActionSelectionTextFilter> TextFilter;
	TSharedPtr<FFIVSEdActionSelectionPinFilter> PinFilter;
	
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
	void Filter_Internal(TSharedPtr<FFIVSEdActionSelectionEntry> Entry, bool bForceAdd);

	/**
	 * Expands all filtered entries
	 */
	void ExpandAll();

	/**
	 * Returns the next action next to the given entry or the entry it self if it is an action.
	 * nullptr if no action is found.
	 */
	TSharedPtr<FFIVSEdActionSelectionEntry> FindNextRelevant(TSharedPtr<FFIVSEdActionSelectionEntry> Entry);
	TSharedPtr<FFIVSEdActionSelectionEntry> FindNextChildRelevant(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry);
	
	/**
	 * Selects the given action
	 */
	void SelectRelevant(const TSharedPtr<FFIVSEdActionSelectionEntry>& Entry);

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
