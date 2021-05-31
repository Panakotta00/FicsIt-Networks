#pragma once

#include "Components/ContentWidget.h"
#include "Components/PanelWidget.h"
#include "FINCopyUUIDButton.generated.h"

UCLASS()
class UFINCopyUUIDButton : public UContentWidget {
	GENERATED_BODY()
private:
	UPROPERTY()
	UWidget* SlotWidget = nullptr;

public:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
	
	void InitSlotWidget(UWidget* SlotIndex);
	
	UFUNCTION()
	void OnCopyUUIDClicked();
};
