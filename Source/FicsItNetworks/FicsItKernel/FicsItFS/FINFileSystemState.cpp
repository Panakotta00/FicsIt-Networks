#include "FINFileSystemState.h"

#include "FGSaveSystem.h"
#include "TimerManager.h"
#include "PlatformFilemanager.h"
#include "UnrealNetwork.h"

AFINFileSystemState::AFINFileSystemState() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"RootComponent");
}

AFINFileSystemState::~AFINFileSystemState() {}

void AFINFileSystemState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINFileSystemState, Usage);
}

void AFINFileSystemState::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);

	if (!Ar.IsSaveGame()) return;
	
	FFileSystemNode node;
	node.NodeType = -1;

	// serialize filesystem devices
	if (Ar.IsSaving()) {
		if (dynamic_cast<FileSystem::MemDevice*>(Device.get())) node.NodeType = 3;
		if (dynamic_cast<FileSystem::DiskDevice*>(Device.get())) node.NodeType = 2;
		if (node.NodeType == 2 || node.NodeType == 3) {
			std::unordered_set<FileSystem::NodeName> nodes = Device->childs("");
			for (FileSystem::NodeName newNode : nodes) {
				TSharedPtr<FFileSystemNode> newSerializeNode = MakeShareable(new FFileSystemNode());
				newSerializeNode->Serialize(Device, newNode);
				node.ChildNodes.Add(FString(newNode.c_str()), newSerializeNode);
			}
		}
	}
	
	Ar << node;
	
	// load devices
	if (Ar.IsLoading()) {
		GetDevice();
		std::string deviceName = TCHAR_TO_UTF8(*ID.ToString());
		node.Deserialize(Device, deviceName);
	}
}

void AFINFileSystemState::BeginPlay() {
	Super::BeginPlay();
	GetDevice();
	
	if (HasAuthority()) GetWorld()->GetTimerManager().SetTimer(UsageUpdateHandler, this, &AFINFileSystemState::UpdateUsage, 1.0f, true);
}

bool AFINFileSystemState::ShouldSave_Implementation() const {
	return true;
}

FileSystem::SRef<FileSystem::Device> AFINFileSystemState::GetDevice() {
	if (!HasAuthority()) return nullptr;
	
	if (!IdCreated) {
		IdCreated = true;
		ID = FGuid::NewGuid();
	}

	if (!Device.isValid()) {
		// get root fs path
		std::filesystem::path root = std::filesystem::absolute(*UFGSaveSystem::GetSaveDirectoryPath());
		root = root / "Computers" / *ID.ToString();
	
		std::filesystem::create_directories(root);

		Device = new FileSystem::DiskDevice(root, Capacity);
	}
	return Device;
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

void AFINFileSystemState::UpdateUsage() {
	FileSystem::SRef<FileSystem::ByteCountedDevice> UsageDevice = GetDevice();
	if (Device) Usage = UsageDevice->getUsed();
	else Usage = 0.0f;
}
