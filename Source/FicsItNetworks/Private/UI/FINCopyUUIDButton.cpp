#include "UI/FINCopyUUIDButton.h"
#include "FINComponentUtility.h"
#include "FicsItKernel/FicsItFS/FINFileSystemState.h"
#include "Reflection/ReflectionHelper.h"
#include "FGInventoryComponent.h"

TSharedRef<SWidget> UFINCopyUUIDButton::RebuildWidget() {
	return GetContent()->TakeWidget();
}

void UFINCopyUUIDButton::InitSlotWidget(UWidget* InSlotWidget) {
	SlotWidget = InSlotWidget;
	
	UClass* StandardButton = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Interface/UI/InGame/-Shared/Widget_StandardButton.Widget_StandardButton_C"));
	check(StandardButton);

	UUserWidget* Button = NewObject<UUserWidget>(this, StandardButton);
	SetContent(Button);
	struct {
		FText Text = FText::FromString("Copy File System ID");
	} params;
	Button->ProcessEvent(Button->FindFunction("SetText"), &params);
	FScriptDelegate Delegate;
	Delegate.BindUFunction(this, TEXT("OnCopyUUIDClicked"));
	FMulticastDelegateProperty* MCDelegate = Cast<FMulticastDelegateProperty>(StandardButton->FindPropertyByName("OnClicked"));
	check(MCDelegate);
	MCDelegate->AddDelegate(Delegate, Button);
}

AFINFileSystemState* UFINCopyUUIDButton::GetFileSystemStateFromSlotWidget(UWidget* InSlot) {
	struct {
		FInventoryStack Stack;
	} Params;
	FReflectionHelper::CallScriptFunction(InSlot, TEXT("GetStack"), &Params);
	AFINFileSystemState* State = Cast<AFINFileSystemState>(Params.Stack.Item.ItemState.Get());
	return State;
}

void UFINCopyUUIDButton::OnCopyUUIDClicked() {
	AFINFileSystemState* State = GetFileSystemStateFromSlotWidget(SlotWidget);
	if (State) {
		UFINComponentUtility::ClipboardCopy(State->ID.ToString());
	}
}
