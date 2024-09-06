#pragma once

#include "Components/ContentWidget.h"
#include "Components/PanelWidget.h"
#include "FINCopyUUIDButton.generated.h"

class AFINFileSystemState;

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

	static AFINFileSystemState* GetFileSystemStateFromSlotWidget(UWidget* InSlot);
	
	UFUNCTION()
	void OnCopyUUIDClicked();
};
