#pragma once

#include "Components/ContentWidget.h"
#include "Components/PanelWidget.h"
#include "FINEditLabel.generated.h"

UCLASS()
class UFINEditLabel : public UContentWidget {
	GENERATED_BODY()
private:
	UPROPERTY()
	UWidget* SlotWidget = nullptr;

public:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	UFUNCTION()
	void InitSlotWidget(UWidget* SlotIndex);

	UFUNCTION()
	void OnLabelChanged(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	static TScriptInterface<IFINLabelContainerInterface> GetLabelContainerFromSlot(UWidget* InSlot);
};
