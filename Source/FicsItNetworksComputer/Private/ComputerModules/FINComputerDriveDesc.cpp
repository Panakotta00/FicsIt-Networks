#include "ComputerModules/FINComputerDriveDesc.h"

#include "FINFileSystemSubsystem.h"
#include "FINLabelContainerInterface.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/TextBlock.h"
#include "FicsItKernel/FicsItFS/FINItemStateFileSystem.h"
#include "FicsItKernel/FicsItFS/Library/Device.h"
#include "Net/UnrealNetwork.h"

UFINComputerDriveDesc::UFINComputerDriveDesc() {
	mStackSize = EStackSize::SS_ONE;
}

void UFINComputerDriveDesc::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UFINComputerDriveDesc, StorageCapacity);
}

FText UFINComputerDriveDesc::GetOverridenItemName_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) {
	FText Name = UFGItemDescriptor::GetItemName(InventoryStack.Item.GetItemClass());
	if (InventoryStack.Item.HasState()) {
		if (const FFINLabelContainerInterface* interface = FFINStructInterfaces::Get().GetInterface<FFINLabelContainerInterface>(InventoryStack.Item.GetItemState())) {
			FString Label = interface->GetLabel();
			if (!Label.IsEmpty()) {
				return FText::FromString(FString::Printf(TEXT("%s - \"%s\""), *Name.ToString(), *Label));
			}
		}
	}
	return Name;
}

FText UFINComputerDriveDesc::GetOverridenItemDescription_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) {
	return FText();
}

UWidget* UFINComputerDriveDesc::CreateDescriptionWidget_Implementation(APlayerController* OwningPlayer, const FInventoryStack& InventoryStack) {
	UClass* progressBar = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Interface/UI/InGame/-Shared/Widget_ProgressBar.Widget_ProgressBar_C"));

	if (!InventoryStack.Item.HasState()) return nullptr;
	auto state = InventoryStack.Item.GetItemState().GetValuePtr<FFINItemStateFileSystem>();
	if (!state) return nullptr;
	FGuid DriveID = state->ID;
	float usage = AFINFileSystemSubsystem::GetFileSystemSubsystem(OwningPlayer)->GetUsage(DriveID);

	UGridPanel* Grid = NewObject<UGridPanel>(OwningPlayer);
	Grid->SetColumnFill(1, 1);
	
	UTextBlock* ProgressPrefix = NewObject<UTextBlock>(OwningPlayer);
	FSlateFontInfo font = ProgressPrefix->GetFont();
	font.Size = 8;
	ProgressPrefix->SetFont(font);
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
	FLinearColor& color = *CastField<FStructProperty>(progressBar->FindPropertyByName("mBarTint"))->ContainerPtrToValuePtr<FLinearColor>(ProgressBar);
	color = FLinearColor(1,1,1);
	UGridSlot* ProgressSlot = Grid->AddChildToGrid(ProgressBar);
	ProgressSlot->SetRow(0);
	ProgressSlot->SetColumn(1);
	ProgressSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
	ProgressSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);

	UTextBlock* UUIDPrefix = NewObject<UTextBlock>(OwningPlayer);
	font = UUIDPrefix->GetFont();
	font.Size = 8;
	UUIDPrefix->SetFont(font);
	UUIDPrefix->SetText(FText::FromString("UUID:"));
	UGridSlot* UUIDPrefixSlot = Grid->AddChildToGrid(UUIDPrefix);
	UUIDPrefixSlot->SetRow(1);
	UUIDPrefixSlot->SetColumn(0);
	UUIDPrefixSlot->SetVerticalAlignment(VAlign_Center);

	UTextBlock* UUID = NewObject<UTextBlock>(OwningPlayer);
	font = UUID->GetFont();
	font.Size = 8;
	UUID->SetFont(font);
	UUID->SetText(FText::FromString(DriveID.ToString()));
	UGridSlot* UUIDSlot = Grid->AddChildToGrid(UUID);
	UUIDSlot->SetRow(1);
	UUIDSlot->SetColumn(1);
	UUIDSlot->SetVerticalAlignment(VAlign_Center);
	UUIDSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
	
	return Grid;
}

void CopyPath(CodersFileSystem::SRef<CodersFileSystem::Device> FromDevice, CodersFileSystem::SRef<CodersFileSystem::Device> ToDevice, CodersFileSystem::Path Path) {
	for (std::string Child : FromDevice->childs(Path)) {
		CodersFileSystem::Path ChildPath = Path / Child;
		CodersFileSystem::SRef<CodersFileSystem::Node> ChildNode = FromDevice->get(ChildPath);
		if (CodersFileSystem::SRef<CodersFileSystem::File> File = ChildNode) {
			CodersFileSystem::SRef<CodersFileSystem::FileStream> InputStream = FromDevice->open(ChildPath, CodersFileSystem::FileMode::INPUT | CodersFileSystem::FileMode::BINARY);
			CodersFileSystem::SRef<CodersFileSystem::FileStream> OutputStream = ToDevice->open(ChildPath, CodersFileSystem::FileMode::OUTPUT | CodersFileSystem::FileMode::BINARY);
			OutputStream->write(CodersFileSystem::FileStream::readAll(InputStream));
			OutputStream->close();
			InputStream->close();
		} else if (CodersFileSystem::SRef<CodersFileSystem::Directory> Dir = ChildNode) {
			ToDevice->createDir(ChildPath);
			CopyPath(FromDevice, ToDevice, ChildPath);
		}
	}
}

bool UFINComputerDriveDesc::CopyData_Implementation(UObject* WorldContext, const FInventoryItem& InFrom, const FInventoryItem& InTo, FInventoryItem& OutItem) {
	TSubclassOf<UFINComputerDriveDesc> DriveClass;
	DriveClass = InTo.GetItemClass();
	if (!IsValid(DriveClass)) return false;

	auto fromState = InFrom.GetItemState().GetValuePtr<FFINItemStateFileSystem>();
	auto toState = InTo.GetItemState().GetValuePtr<FFINItemStateFileSystem>();

	if (!fromState) return false;
	FGuid fromID = fromState->ID;
	FGuid toID;

	OutItem = InTo;
	if (!toState) {
		toID = AFINFileSystemSubsystem::CreateState(GetStorageCapacity(DriveClass), OutItem);
	}

	CodersFileSystem::SRef<CodersFileSystem::Device> FromDevice = AFINFileSystemSubsystem::GetDevice(fromID);
	CodersFileSystem::SRef<CodersFileSystem::Device> ToDevice = AFINFileSystemSubsystem::GetDevice(toID);

	// delete all data in ToDevice
	for (std::string Child : ToDevice->childs("/")) {
		ToDevice->remove(CodersFileSystem::Path(Child), true);
	}

	// copy all data
	CopyPath(FromDevice, ToDevice, "/");
	
	return true;
}

int32 UFINComputerDriveDesc::GetStorageCapacity(TSubclassOf<UFINComputerDriveDesc> drive) {
	return Cast<UFINComputerDriveDesc>(drive->GetDefaultObject())->StorageCapacity;
}

UTexture2D* UFINComputerDriveDesc::GetDriveInventoryImage(TSubclassOf<UFINComputerDriveDesc> drive) {
	return Cast<UFINComputerDriveDesc>(drive->GetDefaultObject())->DriveInventoryImage;
}
