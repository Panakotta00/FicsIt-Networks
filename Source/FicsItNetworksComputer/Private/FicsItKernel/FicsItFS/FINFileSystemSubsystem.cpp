#include "FicsItKernel/FicsItFS/FINFileSystemSubsystem.h"
#include "FicsItkernel/FicsItFS/FINFileSystemState.h"

CodersFileSystem::SRef<CodersFileSystem::Device> AFINFileSystemSubsystem::GetDevice(const FGuid& FileSystemID, bool bInForceUpdate, bool bInForceCreate) {
	if (!FileSystemID.IsValid()) return nullptr;

	CodersFileSystem::SRef<CodersFileSystem::Device> Device;
	CodersFileSystem::SRef<CodersFileSystem::Device>* DevicePtr = Devices.Find(FileSystemID);
	if (DevicePtr) {
		Device = *DevicePtr;
	}

	if (!Device.isValid() || bInForceCreate || bInForceUpdate) {
		FString fsp;
		// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
		if(fsp.IsEmpty()) {
			fsp = FPaths::Combine( FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT( "Saved/" ) TEXT( "SaveGames/" ) );
		}

		// get root fs path
		std::filesystem::path root = std::filesystem::absolute(*fsp);
		root = root / std::filesystem::path(std::string("Computers")) / std::filesystem::path(TCHAR_TO_UTF8(*FileSystemID.ToString()));

		std::filesystem::create_directories(root);

		Device = new CodersFileSystem::DiskDevice(root, 0); // TODO: 1.0: Get Capacity

		if (!Device.isValid() || bInForceUpdate) {
			Devices.Add(FileSystemID, Device);
		}
	}

	return Device;
}

AFINFileSystemSubsystem* AFINFileSystemSubsystem::GetFileSystemSubsystem(UObject* WorldContext) {
#if WITH_EDITOR
	return nullptr;
#endif
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINFileSystemSubsystem>();
}

FGuid AFINFileSystemSubsystem::CreateState(int32 inCapacity, class UFGInventoryComponent* inInventory, int32 inSlot) {
	FFINFileSystemState state;
	state.Capacity = inCapacity;
	state.ID = FGuid::NewGuid();

	inInventory->SetStateOnIndex(inSlot, FFGDynamicStruct(state));

	return state.ID;
}

static FGuid CreateState(int32 inCapacity, FInventoryItem& inItem) {
	FFINFileSystemState state;
	state.Capacity = inCapacity;
	state.ID = FGuid::NewGuid();

	inItem.SetItemState(state);
}