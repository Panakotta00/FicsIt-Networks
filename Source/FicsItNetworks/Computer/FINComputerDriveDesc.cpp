#include "FINComputerDriveDesc.h"


#include "GridPanel.h"
#include "GridSlot.h"
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
	FGuid DriveID = Cast<AFINFileSystemState>(InventoryStack.Item.ItemState.Get())->ID;
	
	UGridPanel* Grid = NewObject<UGridPanel>(OwningPlayer);
	Grid->SetColumnFill(1, 1);
	
	UTextBlock* ProgressPrefix = NewObject<UTextBlock>(OwningPlayer);
	ProgressPrefix->Font.Size = 8;
	ProgressPrefix->SetText(FText::FromString("Usage:"));
	UGridSlot* ProgressPrefixSlot = Grid->AddChildToGrid(ProgressPrefix);
	ProgressPrefixSlot->SetRow(0);
	ProgressPrefixSlot->SetColumn(0);
	ProgressPrefixSlot->SetVerticalAlignment(VAlign_Center);

	UUserWidget* ProgressBar = CreateWidget(OwningPlayer, progressBar);
	struct {
		float start, end, time;
	} params = {0.0, usage, 0.1};
	ProgressBar->ProcessEvent(ProgressBar->FindFunction("LerpBar"), &params);
	FLinearColor& color = *Cast<UStructProperty>(progressBar->FindPropertyByName("mBarTint"))->ContainerPtrToValuePtr<FLinearColor>(ProgressBar);
	color = FLinearColor(1,1,1);
	UGridSlot* ProgressSlot = Grid->AddChildToGrid(ProgressBar);
	ProgressSlot->SetRow(0);
	ProgressSlot->SetColumn(1);
	ProgressSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
	ProgressSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);

	UTextBlock* UUIDPrefix = NewObject<UTextBlock>(OwningPlayer);
	UUIDPrefix->Font.Size = 8;
	UUIDPrefix->SetText(FText::FromString("UUID:"));
	UGridSlot* UUIDPrefixSlot = Grid->AddChildToGrid(UUIDPrefix);
	UUIDPrefixSlot->SetRow(1);
	UUIDPrefixSlot->SetColumn(0);
	UUIDPrefixSlot->SetVerticalAlignment(VAlign_Center);

	UTextBlock* UUID = NewObject<UTextBlock>(OwningPlayer);
	UUID->Font.Size = 8;
	UUID->SetText(FText::FromString(DriveID.ToString()));
	UGridSlot* UUIDSlot = Grid->AddChildToGrid(UUID);
	UUIDSlot->SetRow(1);
	UUIDSlot->SetColumn(1);
	UUIDSlot->SetVerticalAlignment(VAlign_Center);
	UUIDSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
	
	return Grid;
}

int32 UFINComputerDriveDesc::GetStorageCapacity(TSubclassOf<UFINComputerDriveDesc> drive) {
	return Cast<UFINComputerDriveDesc>(drive->GetDefaultObject())->StorageCapacity;
}

UTexture2D* UFINComputerDriveDesc::GetDriveInventoryImage(TSubclassOf<UFINComputerDriveDesc> drive) {
	return Cast<UFINComputerDriveDesc>(drive->GetDefaultObject())->DriveInventoryImage;
}
