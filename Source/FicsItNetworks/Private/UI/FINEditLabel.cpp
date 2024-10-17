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

	UClass* EditLabel = LoadObject<UClass>(NULL, TEXT("/FicsItNetworks/UI/Misc/Widget_FIN_EditLabel.Widget_FIN_EditLabel_C"));
	UUserWidget* Widget = NewObject<UUserWidget>(this, EditLabel);
	SetContent(Widget);
	Widget->Initialize();
	UWidget* TextBoxWidget = Widget->GetWidgetFromName(TEXT("EditLabel"));
	UEditableTextBox* TextBox = Cast<UEditableTextBox>(TextBoxWidget);
	TextBox->OnTextCommitted.AddDynamic(this, &UFINEditLabel::OnLabelChanged);

	TOptional<FString> Label = GetLabelFromSlot(SlotWidget);
	if (Label.IsSet()) {
		TextBox->SetText(FText::FromString(*Label));
	}
}

void UFINEditLabel::OnLabelChanged(const FText& Text, ETextCommit::Type CommitMethod) {
	TOptional<FString> Label = GetLabelFromSlot(SlotWidget);
	if (Label.IsSet()) {
		auto player = GetOwningPlayer<AFGPlayerController>();
		auto rco = Cast<UFINComputerRCO>(player->GetRemoteCallObjectOfClass(UFINComputerRCO::StaticClass()));
		auto inventory = FReflectionHelper::GetObjectPropertyValue<UFGInventoryComponent>(SlotWidget, TEXT("mCachedInventoryComponent"));
		auto slotIndex = FReflectionHelper::GetPropertyValue<FIntProperty>(SlotWidget, TEXT("mSlotIdx"));
		rco->SetLabel(inventory, slotIndex, Text.ToString());
	}
}

TOptional<FString> UFINEditLabel::GetLabelFromSlot(UWidget* InSlot) {
	struct {
		FInventoryStack Stack;
	} Params;
	FReflectionHelper::CallScriptFunction(InSlot, TEXT("GetStack"), &Params);

	if (!Params.Stack.Item.HasState()) return {};

	const FFINLabelContainerInterface* container = FFINStructInterfaces::Get().GetInterface<FFINLabelContainerInterface>(Params.Stack.Item.GetItemState());
	if (container) return container->GetLabel();
	return {};
}
