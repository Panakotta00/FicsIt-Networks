#include "FINFileSystemState.h"

#include "FGSaveSystem.h"

#include "PlatformFilemanager.h"

AFINFileSystemState::AFINFileSystemState() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"RootComponent");
}

AFINFileSystemState::~AFINFileSystemState() {}

void AFINFileSystemState::BeginPlay() {
	if (!IdCreated) {
		IdCreated = true;
		ID = FGuid::NewGuid();
	}
}

bool AFINFileSystemState::ShouldSave_Implementation() const {
	return true;
}

FileSystem::SRef<FileSystem::Device> AFINFileSystemState::createDevice() const {
	// get root fs path
	std::filesystem::path root = std::filesystem::absolute(*UFGSaveSystem::GetSaveDirectoryPath());
	root = root / "Computers" / *ID.ToString();
	
	std::filesystem::create_directories(root);

	return new FileSystem::DiskDevice(root, Capacity);
}

AFINFileSystemState* AFINFileSystemState::CreateState(UObject* WorldContextObject, int32 inCapacity, UFGInventoryComponent* inInventory, int32 inSlot) {
	FVector loc = FVector::ZeroVector;
	FRotator rot = FRotator::ZeroRotator;
	AFINFileSystemState* efs = WorldContextObject->GetWorld()->SpawnActor<AFINFileSystemState>(loc, rot);
	efs->Capacity = inCapacity;

	FSharedInventoryStatePtr statePtr = FSharedInventoryStatePtr::MakeShared(efs);
	inInventory->SetStateOnIndex(inSlot, statePtr);
	return efs;
}
