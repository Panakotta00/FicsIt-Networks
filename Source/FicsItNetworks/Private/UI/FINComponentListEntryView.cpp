#include "UI/FINComponentListEntryView.h"

#include "Blueprint/IUserListEntry.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

void UFINComponentListEntryView::NativeConstruct()
{
	Super::NativeConstruct();

	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if (UButton* Button = Cast<UButton>(Widget))
			{
				Button->OnClicked.AddUniqueDynamic(this, &UFINComponentListEntryView::SelectOwningListItem);
			}
		});
	}
}

FReply UFINComponentListEntryView::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SelectOwningListItem();
	}

	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

void UFINComponentListEntryView::SelectOwningListItem()
{
	if (UObject* ListItem = GetListItem<UObject>())
	{
		if (UListView* ListView = Cast<UListView>(GetOwningListView()))
		{
			ListView->SetSelectedItem(ListItem);
		}
	}
}
