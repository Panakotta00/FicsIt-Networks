#include "UI/FINComponentListEntryView.h"

#include "Blueprint/IUserListEntry.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "UObject/ScriptInterface.h"
#include "UObject/UnrealType.h"

void UFINComponentListEntryView::NativeConstruct()
{
	Super::NativeConstruct();

	static const FName OpenInButtonName(TEXT("Button_35"));
	if (WidgetTree)
	{
		WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			if (UButton* Button = Cast<UButton>(Widget); Button && Button->GetFName() != OpenInButtonName)
			{
				Button->OnClicked.AddUniqueDynamic(this, &UFINComponentListEntryView::SelectOwningListItem);
			}
		});
	}
}

void UFINComponentListEntryView::SelectOwningListItem()
{
	UObject* ListItem = GetListItem<UObject>();
	if (ListItem)
	{
		if (UListView* ListView = Cast<UListView>(GetOwningListView()))
		{
			ListView->SetSelectedItem(ListItem);
		}
	}

	UObject* Component = GetListItemComponent();
	UObject* ComponentDebugWidget = FindComponentDebugWidget();
	UFunction* SetComponentFunction = ComponentDebugWidget ? ComponentDebugWidget->FindFunction(TEXT("SetComponent")) : nullptr;
	if (Component && SetComponentFunction)
	{
		TArray<uint8> Parameters;
		Parameters.SetNumZeroed(SetComponentFunction->ParmsSize);
		SetFunctionComponentParameter(SetComponentFunction, Parameters.GetData(), Component);
		ComponentDebugWidget->ProcessEvent(SetComponentFunction, Parameters.GetData());
	}
}

UObject* UFINComponentListEntryView::GetListItemComponent() const
{
	UObject* ListItem = GetListItem<UObject>();
	if (!ListItem)
	{
		return nullptr;
	}

	FProperty* ComponentProperty = ListItem->GetClass()->FindPropertyByName(TEXT("Component"));
	if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(ComponentProperty))
	{
		return ObjectProperty->GetObjectPropertyValue_InContainer(ListItem);
	}
	if (FInterfaceProperty* InterfaceProperty = CastField<FInterfaceProperty>(ComponentProperty))
	{
		if (const FScriptInterface* InterfaceValue = InterfaceProperty->GetPropertyValuePtr_InContainer(ListItem))
		{
			return InterfaceValue->GetObject();
		}
	}

	return nullptr;
}

UObject* UFINComponentListEntryView::FindComponentDebugWidget() const
{
	for (UObject* Outer = GetOwningListView(); Outer; Outer = Outer->GetOuter())
	{
		if (Outer != this && Outer->FindFunction(TEXT("SetComponent")))
		{
			return Outer;
		}
	}

	for (UObject* Outer = GetOuter(); Outer; Outer = Outer->GetOuter())
	{
		if (Outer != this && Outer->FindFunction(TEXT("SetComponent")))
		{
			return Outer;
		}
	}

	return nullptr;
}

void UFINComponentListEntryView::SetFunctionComponentParameter(UFunction* Function, void* Parameters, UObject* Component) const
{
	for (TFieldIterator<FProperty> It(Function); It; ++It)
	{
		FProperty* Property = *It;
		if (!Property->HasAnyPropertyFlags(CPF_Parm) || Property->HasAnyPropertyFlags(CPF_ReturnParm))
		{
			continue;
		}

		if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
		{
			ObjectProperty->SetObjectPropertyValue_InContainer(Parameters, Component);
			return;
		}
		if (FInterfaceProperty* InterfaceProperty = CastField<FInterfaceProperty>(Property))
		{
			FScriptInterface* InterfaceValue = InterfaceProperty->GetPropertyValuePtr_InContainer(Parameters);
			InterfaceValue->SetObject(Component);
			InterfaceValue->SetInterface(Component->GetInterfaceAddress(InterfaceProperty->InterfaceClass));
			return;
		}
	}
}
