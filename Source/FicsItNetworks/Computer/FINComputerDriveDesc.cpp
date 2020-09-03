#include "FINComputerDriveDesc.h"

#include "HorizontalBox.h"
#include "HorizontalBoxSlot.h"
#include "TextBlock.h"
#include "UnrealNetwork.h"
#include "FicsItKernel/FicsItFS/FINFileSystemState.h"

UFINComputerDriveDesc::UFINComputerDriveDesc() {
	mStackSize = EStackSize::SS_ONE;
}

void UFINComputerDriveDesc::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFINComputerDriveDesc, StorageCapacity);
}

FText UFINComputerDriveDesc::GetOverridenItemName_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) {
	return UFGItemDescriptor::GetItemName(InventoryStack.Item.ItemClass);
}

FText UFINComputerDriveDesc::GetOverridenItemDescription_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) {
	return FText();
}

UWidget* UFINComputerDriveDesc::CreateDescriptionWidget_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) {
	UClass* progressBar = LoadObject<UClass>(NULL, TEXT("/Game/FactoryGame/Interface/UI/InGame/-Shared/Widget_ProgressBar.Widget_ProgressBar_C"));

	if (!InventoryStack.Item.ItemState.IsValid()) return nullptr;
	float usage = Cast<AFINFileSystemState>(InventoryStack.Item.ItemState.Get())->Usage;
	UHorizontalBox* horizontal = NewObject<UHorizontalBox>(OwningPlayer);
	UTextBlock* prefix = NewObject<UTextBlock>(OwningPlayer);
	prefix->Font.Size = 8;
	prefix->SetText(FText::FromString("Usage:"));
	horizontal->AddChild(prefix);
	Cast<UHorizontalBoxSlot>(prefix->Slot)->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
	UUserWidget* progress = CreateWidget(OwningPlayer, progressBar);
	struct {
		float start, end, time;
	} params = {0.0, usage, 0.1};
	progress->ProcessEvent(progress->FindFunction("LerpBar"), &params);
	FLinearColor& color = *Cast<UStructProperty>(progressBar->FindPropertyByName("mBarTint"))->ContainerPtrToValuePtr<FLinearColor>(progress);
	color = FLinearColor(1,1,1);
	horizontal->AddChild(progress);
	UHorizontalBoxSlot* progressSlot = Cast<UHorizontalBoxSlot>(progress->Slot);
	progressSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
	progressSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
	progressSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	return horizontal;
}

int32 UFINComputerDriveDesc::GetStorageCapacity(TSubclassOf<UFINComputerDriveDesc> drive) {
	return Cast<UFINComputerDriveDesc>(drive->GetDefaultObject())->StorageCapacity;
}

UTexture2D* UFINComputerDriveDesc::GetDriveInventoryImage(TSubclassOf<UFINComputerDriveDesc> drive) {
	return Cast<UFINComputerDriveDesc>(drive->GetDefaultObject())->DriveInventoryImage;
}
