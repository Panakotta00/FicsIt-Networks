#pragma once

#include "FINReflectionUIContext.h"
#include "FINReflectionUIStyle.h"
#include "Components/Widget.h"
#include "FINReflectionUI.generated.h"

class SFINReflectionUI : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFINReflectionUI) :
		_Style(&FFINReflectionUIStyleStruct::GetDefault()) {}
		SLATE_ATTRIBUTE(const FFINReflectionUIStyleStruct*, Style)
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs);

private:
	SSplitter::FSlot* Slot;
	TArray<TSharedPtr<FFINReflectionUIEntry>> Filtered;
	TSharedPtr<STreeView<TSharedPtr<FFINReflectionUIEntry>>> Tree;
	TSharedPtr<SEditableTextBox> SearchBox;
	bool bInitFocus = true;

public:
	FFINReflectionUIContext Context;
	
	SFINReflectionUI();

	void FilterCache(const FString& Filter);

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual bool IsInteractable() const override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
};

UCLASS(BlueprintType)
class UFINReflectionUI : public UWidget {
	GENERATED_BODY()
private:
	TSharedPtr<SFINReflectionUI> Container = nullptr;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=( DisplayName="Style" ))
	FFINReflectionUIStyleStruct Style;

	UFINReflectionUI();
	
	// Begin UWidget
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;	
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget

	/**
	 * Sets the currently selected typer of the reflection viewer
	 */
	UFUNCTION(BlueprintCallable, Category="Reflection|UI")
	void NavigateTo(UFINStruct* InStruct);

	/**
	 * Sets if entry lists should also show the entries of super structs/classes
	 */
	UFUNCTION(BlueprintCallable, Category="Reflection|UI")
	void SetShowRecursive(bool bInShowRecursive);
};
