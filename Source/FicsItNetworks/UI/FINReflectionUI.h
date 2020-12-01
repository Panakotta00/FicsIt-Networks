#pragma once

#include "FINReflectionUIContext.h"
#include "FINReflectionUIStyle.h"
#include "HorizontalBox.h"
#include "SCompoundWidget.h"
#include "Widget.h"
#include "FINReflectionUI.generated.h"

class SFINReflectionUI : public SCompoundWidget {
	SLATE_BEGIN_ARGS(SFINReflectionUI) :
		_Style(&FFINReflectionUIStyleStruct::GetDefault()) {}
		SLATE_ATTRIBUTE(const FFINReflectionUIStyleStruct*, Style)
	SLATE_END_ARGS()
public:
	void Construct(const FArguments& InArgs);

private:
	SHorizontalBox::FSlot* Slot;
	TArray<TSharedPtr<FFINReflectionUIEntry>> Filtered;
	TSharedPtr<STreeView<TSharedPtr<FFINReflectionUIEntry>>> Tree;

public:
	FFINReflectionUIContext Context;
	
	SFINReflectionUI();

	void FilterCache(const FString& Filter);
};

UCLASS(BlueprintType)
class UFINReflectionUI : public UWidget {
	GENERATED_BODY()
private:
	TSharedPtr<SFINReflectionUI> Container = nullptr;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=( DisplayName="Style" ))
	FFINReflectionUIStyleStruct Style;
	
	// Begin UWidget
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;	
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End UWidget

	/**
	 * Sets the currently selected typer of the reflection viewer
	 */
	UFUNCTION(BlueprintCallable, Category="Reflection|UI")
	void SetSelected(UFINStruct* InStruct);
};
