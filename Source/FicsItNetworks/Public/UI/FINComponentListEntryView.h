#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"

#include "FINComponentListEntryView.generated.h"

UCLASS()
class FICSITNETWORKS_API UFINComponentListEntryView : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION()
	void SelectOwningListItem();

private:
	UObject* GetListItemComponent() const;
	UObject* FindComponentDebugWidget() const;
	void SetFunctionComponentParameter(UFunction* Function, void* Parameters, UObject* Component) const;
};
