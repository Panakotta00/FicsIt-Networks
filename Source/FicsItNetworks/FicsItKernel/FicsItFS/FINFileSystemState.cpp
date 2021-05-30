#include "FINFileSystemState.h"
#include "FGSaveSystem.h"
#include "TimerManager.h"
#include "FicsItNetworks/FicsItNetworksModule.h"

AFINFileSystemState::AFINFileSystemState() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"RootComponent");
}

AFINFileSystemState::~AFINFileSystemState() {}

void AFINFileSystemState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINFileSystemState, Usage);
}

void AFINFileSystemState::SerializePath(CodersFileSystem::SRef<CodersFileSystem::Device> SerializeDevice, FStructuredArchive::FRecord Record, CodersFileSystem::Path Path) {
	std::unordered_set<CodersFileSystem::NodeName> childs;
	std::unordered_set<CodersFileSystem::NodeName>::iterator childIterator;
	int ChildNodeNum;
	if (Record.GetUnderlyingArchive().IsSaving()) {
		childs = SerializeDevice->childs(Path);
		childIterator = childs.begin();
		ChildNodeNum = childs.size();
	}
	FStructuredArchive::FArray ChildNodes = Record.EnterArray(SA_FIELD_NAME(TEXT("ChildNodes")), ChildNodeNum);
	for (int i = 0; i < ChildNodeNum; ++i) {
		FStructuredArchive::FRecord Child = ChildNodes.EnterElement().EnterRecord();
		FString UChildName;
		if (Record.GetUnderlyingArchive().IsSaving()) {
			UChildName = UTF8_TO_TCHAR((childIterator++)->c_str());
		}
		Child.EnterField(SA_FIELD_NAME(TEXT("Name"))) << UChildName;
		std::string stdChildName = TCHAR_TO_UTF8(*UChildName);

		CodersFileSystem::SRef<CodersFileSystem::Node> ChildNode = SerializeDevice->get(Path / stdChildName);
		int Type = 0;
		if (dynamic_cast<CodersFileSystem::File*>(ChildNode.get())) Type = 1;
		if (dynamic_cast<CodersFileSystem::Directory*>(ChildNode.get())) Type = 2;
		Child.EnterField(SA_FIELD_NAME(TEXT("Type"))) << Type;
		if (Type == 1) {
			FStructuredArchive::FSlot Content = Child.EnterField(SA_FIELD_NAME(TEXT("FileContent")));
			if (Record.GetUnderlyingArchive().IsLoading()) {
				CodersFileSystem::SRef<CodersFileSystem::FileStream> Stream = SerializeDevice->open(Path / stdChildName, CodersFileSystem::OUTPUT | CodersFileSystem::TRUNC | CodersFileSystem::BINARY);
				FString Data;
				Content << Data;
				FTCHARToUTF8 Convert(*Data, Data.Len());
				Stream->write(std::string(Convert.Get(), Convert.Length()));
				Stream->flush();
				Stream->close();
            } else if (Record.GetUnderlyingArchive().IsSaving()) {
            	CodersFileSystem::SRef<CodersFileSystem::FileStream> Stream = SerializeDevice->open(Path / stdChildName, CodersFileSystem::INPUT | CodersFileSystem::BINARY);
            	std::string RawData = Stream->readAll();
            	Stream->close();
            	FUTF8ToTCHAR Convert(RawData.c_str(), RawData.length());
            	FString Data(Convert.Length(), Convert.Get());
            	Content << Data;
            }
		} else if (Type == 2) {
			SerializeDevice->createDir(Path / stdChildName);
			SerializePath(SerializeDevice, Child, Path / stdChildName);
		}
	}
}

void AFINFileSystemState::Serialize(FStructuredArchive::FRecord Record) {
	Super::Serialize(Record);

	if (!Record.GetUnderlyingArchive().IsSaveGame()) return;

	int32 OldCapacity = Capacity;
	Capacity = 0;
	CodersFileSystem::SRef<CodersFileSystem::Device> SerializeDevice = GetDevice(false, true);
	Capacity = OldCapacity;

	if (Record.GetUnderlyingArchive().IsLoading()) {
		SerializeDevice->remove("/", true);
	}
	
	FStructuredArchive::FSlot RootNode = Record.EnterField(SA_FIELD_NAME(TEXT("RootNode")));
	if (!bUseOldSerialization) {
		SerializePath(SerializeDevice, RootNode.EnterRecord(), "/");
	} else {
		Serialize_DEPRECATED(RootNode.GetUnderlyingArchive());
		bUseOldSerialization = false;
	}
}

void AFINFileSystemState::BeginPlay() {
	Super::BeginPlay();
	GetDevice();
	
	if (HasAuthority()) GetWorld()->GetTimerManager().SetTimer(UsageUpdateHandler, this, &AFINFileSystemState::UpdateUsage, 1.0f, true);
}

void AFINFileSystemState::PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	bUseOldSerialization = gameVersion < 150216;
}

bool AFINFileSystemState::ShouldSave_Implementation() const {
	return true;
}

CodersFileSystem::SRef<CodersFileSystem::Device> AFINFileSystemState::GetDevice(bool bInForceUpdate, bool bInForceCreate) {
	if (!HasAuthority()) return nullptr;
	
	if (!IdCreated) {
		IdCreated = true;
		ID = FGuid::NewGuid();
	}

	CodersFileSystem::SRef<CodersFileSystem::Device> NewDevice = Device;
	if (!Device.isValid() || bInForceCreate || bInForceUpdate) {
		FString fsp;
		// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
		if(fsp.IsEmpty()) {
			fsp = FPaths::Combine( FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT( "Saved/" ) TEXT( "SaveGames/" ) );
		}

		// get root fs path
		std::filesystem::path root = std::filesystem::absolute(*fsp);
		root = root / std::filesystem::path(std::string("Computers")) / std::filesystem::path(TCHAR_TO_UTF8(*ID.ToString()));
	
		std::filesystem::create_directories(root);

		NewDevice = new CodersFileSystem::DiskDevice(root, Capacity);

		if (!Device.isValid() || bInForceUpdate) Device = NewDevice;
	}
	return NewDevice;
}

AFINFileSystemState* AFINFileSystemState::CreateState(UObject* WorldContextObject, int32 inCapacity, UFGInventoryComponent* inInventory, int32 inSlot) {
	FVector loc = FVector::ZeroVector;
	FRotator rot = FRotator::ZeroRotator;
	AFINFileSystemState* efs = WorldContextObject->GetWorld()->SpawnActor<AFINFileSystemState>(loc, rot);
	efs->Capacity = inCapacity;

	FSharedInventoryStatePtr statePtr = FSharedInventoryStatePtr::MakeShared((AActor*)efs);
	inInventory->SetStateOnIndex(inSlot, statePtr);
	return efs;
}

void AFINFileSystemState::UpdateUsage() {
	CodersFileSystem::SRef<CodersFileSystem::ByteCountedDevice> UsageDevice = GetDevice();
	if (Device) Usage = UsageDevice->getUsed();
	else Usage = 0.0f;
}

void AFINFileSystemState::Serialize_DEPRECATED(FArchive& Ar) {
	if (!Ar.IsSaveGame()) return;
	
	FFileSystemNode node;
	node.NodeType = -1;

	// serialize filesystem devices
	if (Ar.IsSaving()) {
		if (dynamic_cast<CodersFileSystem::MemDevice*>(Device.get())) node.NodeType = 3;
		if (dynamic_cast<CodersFileSystem::DiskDevice*>(Device.get())) node.NodeType = 2;
		if (node.NodeType == 2 || node.NodeType == 3) {
			std::unordered_set<CodersFileSystem::NodeName> nodes = Device->childs("");
			for (CodersFileSystem::NodeName newNode : nodes) {
				TSharedPtr<FFileSystemNode> newSerializeNode = MakeShareable(new FFileSystemNode());
				newSerializeNode->Serialize(Device, newNode.c_str());
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
