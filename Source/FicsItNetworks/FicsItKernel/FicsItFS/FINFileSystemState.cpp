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

/*bool AFINFileSystemState::ShouldSaveState() const {
	return true;
}*/

FileSystem::SRef<FileSystem::Device> AFINFileSystemState::createDevice() const {
	// get root fs path
	std::filesystem::path root = std::filesystem::absolute(*UFGSaveSystem::GetSaveDirectoryPath());
	root = root / "Computers" / *ID.ToString();
	
	std::filesystem::create_directories(root);

	return new FileSystem::DiskDevice(root, Capacity);
}

AFINFileSystemState* AFINFileSystemState::CreateState(UObject* WorldContextObject, int32 inCapacity, UFGInventoryComponent* inInventory, int32 inSlot) {
	FVector loc{0,0,0};
	FRotator rot{0,0,0};
	auto efs = WorldContextObject->GetWorld()->SpawnActor<AFINFileSystemState>(loc, rot);
	efs->Capacity = inCapacity;

	auto statePtr = FSharedInventoryStatePtr::MakeShared(efs);
	inInventory->SetStateOnIndex(inSlot, statePtr);
	return efs;
}
