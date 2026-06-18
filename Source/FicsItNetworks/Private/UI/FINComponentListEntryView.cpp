#include "UI/FINComponentListEntryView.h"

#include "Blueprint/IUserListEntry.h"
#include "Components/ListView.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

FReply UFINComponentListEntryView::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (UObject* ListItem = GetListItem<UObject>())
		{
			if (UListView* ListView = Cast<UListView>(GetOwningListView()))
			{
				ListView->SetSelectedItem(ListItem);
			}
		}
	}

	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}
