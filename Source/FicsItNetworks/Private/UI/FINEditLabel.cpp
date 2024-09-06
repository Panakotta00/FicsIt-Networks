#include "UI/FINEditLabel.h"

#include "FGPlayerController.h"
#include "FINComputerRCO.h"
#include "FINLabelContainerInterface.h"
#include "Components/EditableTextBox.h"
#include "Reflection/ReflectionHelper.h"
#include "UI/FINCopyUUIDButton.h"

TSharedRef<SWidget> UFINEditLabel::RebuildWidget() {
	return GetContent()->TakeWidget();
}

void UFINEditLabel::InitSlotWidget(UWidget* InSlotWidget) {
	SlotWidget = InSlotWidget;

	UClass* EditLabel = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/UI/Widget_FIN_EditLabel.Widget_FIN_EditLabel_C"));
	UUserWidget* Widget = NewObject<UUserWidget>(this, EditLabel);
	SetContent(Widget);
	Widget->Initialize();
	UWidget* TextBoxWidget = Widget->GetWidgetFromName(TEXT("EditLabel"));
	UEditableTextBox* TextBox = Cast<UEditableTextBox>(TextBoxWidget);
	TextBox->OnTextCommitted.AddDynamic(this, &UFINEditLabel::OnLabelChanged);

	TScriptInterface<IFINLabelContainerInterface> LabelContainer = GetLabelContainerFromSlot(SlotWidget);
	if (LabelContainer) {
		TextBox->SetText(FText::FromString(IFINLabelContainerInterface::Execute_GetLabel(LabelContainer.GetObject())));
	}
}

UE_DISABLE_OPTIMIZATION_SHIP
void UFINEditLabel::OnLabelChanged(const FText& Text, ETextCommit::Type CommitMethod) {
	TScriptInterface<IFINLabelContainerInterface> LabelContainer = GetLabelContainerFromSlot(SlotWidget);
	if (LabelContainer) {
		auto player = GetOwningPlayer<AFGPlayerController>();
		auto rco = Cast<UFINComputerRCO>(player->GetRemoteCallObjectOfClass(UFINComputerRCO::StaticClass()));
		rco->SetLabel(LabelContainer.GetObject(), Text.ToString());
	}
}
UE_ENABLE_OPTIMIZATION_SHIP

TScriptInterface<IFINLabelContainerInterface> UFINEditLabel::GetLabelContainerFromSlot(UWidget* InSlot) {
	struct {
		FInventoryStack Stack;
	} Params;
	FReflectionHelper::CallScriptFunction(InSlot, TEXT("GetStack"), &Params);
	if (!Params.Stack.Item.ItemState.IsValid()) return nullptr;
	UObject* State = Params.Stack.Item.ItemState.Get();
	if (State->Implements<UFINLabelContainerInterface>()) {
		return State;
	}
	return nullptr;
}
