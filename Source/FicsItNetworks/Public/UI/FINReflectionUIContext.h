#pragma once

#include "FINReflectionUIStyle.h"

class FFINReflectionUIContext;
class UFINSignal;
class UFINClass;
class UFINStruct;
class UFINFunction;
class UFINProperty;

enum EFINReflectionFilterState {
	FIN_Ref_Filter_None,
	FIN_Ref_Filter_Self,
	FIN_Ref_Filter_Child
};

TSharedRef<SWidget> GenerateDataTypeIcon(UFINProperty* Prop, FFINReflectionUIContext* Context);
TSharedRef<SWidget> GeneratePropTypeIcon(UFINProperty* Prop, FFINReflectionUIContext* Context);
TSharedRef<SWidget> GenerateFuncTypeIcon(UFINFunction* Func, FFINReflectionUIContext* Context);

class FFINReflectionUIEntry;

DECLARE_MULTICAST_DELEGATE_OneParam(FFINReflectionUISelectionChanged, FFINReflectionUIEntry*)

class FFINReflectionUIFilter {
private:
	TArray<FString> Tokens;	
public:
	FFINReflectionUIFilter(FString Filter);
	
	bool PassesFilter(const FString& String) const;
};

class FFINReflectionUIEntry : public TSharedFromThis<FFINReflectionUIEntry> {
protected:
	virtual void LoadChildren() {};
	bool bUpdateChildren = true;
	
public:
	TWeakPtr<FFINReflectionUIEntry> Parent;
	FFINReflectionUIContext* Context;

	FFINReflectionUIEntry(const TWeakPtr<FFINReflectionUIEntry>& Parent, FFINReflectionUIContext* Context) : Parent(Parent), Context(Context) {}
	virtual ~FFINReflectionUIEntry() = default;
	
	virtual TArray<TSharedPtr<FFINReflectionUIEntry>> GetChildren() {
		return TArray<TSharedPtr<FFINReflectionUIEntry>>();
	}
	virtual TSharedRef<SWidget> GetDetailsWidget() = 0;
	virtual TSharedRef<SWidget> GetShortPreview() = 0;
	virtual TSharedRef<SWidget> GetPreview() = 0;
	virtual TSharedRef<SWidget> GetLink();
	virtual EFINReflectionFilterState ApplyFilter(const FFINReflectionUIFilter& Filter) = 0;
	
	void UpdateChildren(bool bForce = false);
};

class FFINReflectionUIStruct : public FFINReflectionUIEntry {
protected:
	UFINStruct* Struct;
	TArray<TSharedPtr<FFINReflectionUIEntry>> Filtered;

	virtual void LoadChildren() override;

public:
	TArray<TSharedPtr<FFINReflectionUIEntry>> Attributes;
	TArray<TSharedPtr<FFINReflectionUIEntry>> Functions;
	
	FFINReflectionUIStruct(const TWeakPtr<FFINReflectionUIEntry>& Parent, UFINStruct* Struct, FFINReflectionUIContext* Context);
	
	virtual TArray<TSharedPtr<FFINReflectionUIEntry>> GetChildren() override { UpdateChildren(); return Filtered; }
	virtual TSharedRef<SWidget> GetDetailsWidget() override;
	virtual TSharedRef<SWidget> GetShortPreview() override;
	virtual TSharedRef<SWidget> GetPreview() override;
	virtual EFINReflectionFilterState ApplyFilter(const FFINReflectionUIFilter& Filter) override;

	UFINStruct* GetStruct() const { return Struct; }
};

class FFINReflectionUIClass : public FFINReflectionUIStruct {
protected:
	virtual void LoadChildren() override;
	
public:
	TArray<TSharedPtr<FFINReflectionUIEntry>> Signals;
	
	FFINReflectionUIClass(const TWeakPtr<FFINReflectionUIEntry>& Parent, UFINClass* Class, FFINReflectionUIContext* Context);
	
	virtual TSharedRef<SWidget> GetDetailsWidget() override;
	virtual EFINReflectionFilterState ApplyFilter(const FFINReflectionUIFilter& Filter) override;

	UFINClass* GetClass() const;
};

class FFINReflectionUIProperty : public FFINReflectionUIEntry {
private:
	UFINProperty* Property;

public:
	FFINReflectionUIProperty(const TWeakPtr<FFINReflectionUIEntry>& Parent, UFINProperty* Property, FFINReflectionUIContext* Context) : FFINReflectionUIEntry(Parent, Context), Property(Property) {}
	
	virtual TSharedRef<SWidget> GetDetailsWidget() override;
	virtual TSharedRef<SWidget> GetShortPreview() override;
	virtual TSharedRef<SWidget> GetPreview() override;
	virtual EFINReflectionFilterState ApplyFilter(const FFINReflectionUIFilter& Filter) override;
};

class FFINReflectionUIFunction : public FFINReflectionUIEntry {
private:
	UFINFunction* Function;

protected:
	virtual void LoadChildren() override;

public:
	TArray<TSharedPtr<FFINReflectionUIEntry>> Parameters;
	
	FFINReflectionUIFunction(const TWeakPtr<FFINReflectionUIEntry>& Parent, UFINFunction* Function, FFINReflectionUIContext* Context) : FFINReflectionUIEntry(Parent, Context), Function(Function) {}

	virtual TSharedRef<SWidget> GetDetailsWidget() override;
	virtual TSharedRef<SWidget> GetShortPreview() override;
	virtual TSharedRef<SWidget> GetPreview() override;
	virtual EFINReflectionFilterState ApplyFilter(const FFINReflectionUIFilter& Filter) override;
};

class FFINReflectionUISignal : public FFINReflectionUIEntry {
private:
	UFINSignal* Signal;

protected:
	virtual void LoadChildren() override;

public:
	TArray<TSharedPtr<FFINReflectionUIEntry>> Parameters;
	
	FFINReflectionUISignal(const TWeakPtr<FFINReflectionUIEntry>& Parent, UFINSignal* Signal, FFINReflectionUIContext* Context) : FFINReflectionUIEntry(Parent, Context), Signal(Signal) {}

	virtual TSharedRef<SWidget> GetDetailsWidget() override;
	virtual TSharedRef<SWidget> GetShortPreview() override;
	virtual TSharedRef<SWidget> GetPreview() override;
	virtual EFINReflectionFilterState ApplyFilter(const FFINReflectionUIFilter& Filter) override;
};

class FFINReflectionUIContext : TSharedFromThis<FFINReflectionUIContext> {
private:
	FFINReflectionUIEntry* SelectedEntry = nullptr;
	TArray<FFINReflectionUIEntry*> NavigationHistory;
	int NavigationHistoryIndex = 0;
	const int NAVIGATION_HISTORY_MAX = 32;
	bool bShowRecursive = false;

	void SetSelected(FFINReflectionUIEntry* Entry);

public:
	TAttribute<const FFINReflectionUIStyleStruct*> Style;
	TArray<TSharedPtr<FFINReflectionUIEntry>> Entries;
	TMap<UFINStruct*, TSharedPtr<FFINReflectionUIStruct>> Structs;
	FString FilterString;
	
	FFINReflectionUISelectionChanged OnSelectionChanged;

	FFINReflectionUIContext();

	void NavigateNext();
	void NavigatePrevious();
	void NavigateTo(FFINReflectionUIEntry* Entry);
	FFINReflectionUIEntry* GetSelected() const;
	void SetShowRecursive(bool bInShowRecursive);
	bool GetShowRecursive() const { return bShowRecursive; }
};
