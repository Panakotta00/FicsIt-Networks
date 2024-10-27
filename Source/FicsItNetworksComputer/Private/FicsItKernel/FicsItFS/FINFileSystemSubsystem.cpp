#include "FicsItKernel/FicsItFS/FINFileSystemSubsystem.h"

#include "App.h"
#include "Device.h"
#include "FGInventoryComponent.h"
#include "FicsItFileSystem.h"
#include "Paths.h"
#include "SubsystemActorManager.h"
#include "Engine/Engine.h"
#include "FicsItKernel/FicsItFS/FINItemStateFileSystem.h"

TMap<FGuid, TSharedRef<CodersFileSystem::Device>> AFINFileSystemSubsystem::Devices;

TSharedPtr<CodersFileSystem::Device> AFINFileSystemSubsystem::GetDevice(const FGuid& FileSystemID, bool bInForceUpdate, bool bInForceCreate) {
	if (!FileSystemID.IsValid()) return nullptr;

	TSharedPtr<CodersFileSystem::Device> Device;
	TSharedRef<CodersFileSystem::Device>* DevicePtr = Devices.Find(FileSystemID);
	if (DevicePtr) {
		Device = *DevicePtr;
	}

	if (!Device.IsValid() || bInForceCreate || bInForceUpdate) {
		FString fsp;
		// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
		if(fsp.IsEmpty()) {
			fsp = FPaths::Combine( FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT( "Saved/" ) TEXT( "SaveGames/" ) );
		}

		fsp = FPaths::Combine(fsp, "Computers", FileSystemID.ToString());

		// get root fs path
		std::filesystem::path root = std::filesystem::absolute(*fsp);

		std::filesystem::create_directories(root);

		Device = MakeShared<CodersFileSystem::DiskDevice>(root, 0); // TODO: 1.0: Get Capacity

		Devices.Add(FileSystemID, Device.ToSharedRef());
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
	FFINItemStateFileSystem state;
	state.Capacity = inCapacity;
	state.ID = FGuid::NewGuid();

	inInventory->SetStateOnIndex(inSlot, FFGDynamicStruct(state));

	return state.ID;
}

FGuid AFINFileSystemSubsystem::CreateState(int32 inCapacity, FInventoryItem& inItem) {
	FFINItemStateFileSystem state;
	state.Capacity = inCapacity;
	state.ID = FGuid::NewGuid();

	inItem.SetItemState(FFGDynamicStruct(state));

	return state.ID;
}
