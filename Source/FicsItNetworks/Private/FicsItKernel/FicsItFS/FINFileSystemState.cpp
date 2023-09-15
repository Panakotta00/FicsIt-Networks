#include "FicsItKernel/FicsItFS/FINFileSystemState.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SViewport.h"

AFINFileSystemState::AFINFileSystemState() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(L"RootComponent");
}

AFINFileSystemState::~AFINFileSystemState() {}

void AFINFileSystemState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINFileSystemState, Usage);
}

int AskForDiskOrSave(FString Name) {
	int KeepDisk = 0;
	TSharedRef<SWindow> Window = SNew(SWindow)
		.HasCloseButton(false)
		.ActivationPolicy(EWindowActivationPolicy::Always)
		.FocusWhenFirstShown(true)
		.SizingRule(ESizingRule::Autosized)
		.Title(FText::FromString(FString::Printf(TEXT("FileSystem Content difference in FS '%s' found!"), *Name)))
		.Content()[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1)[
				SNew(SBox)
				.MinDesiredHeight(100)
				.MinDesiredWidth(100)[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("It appears as if the disk filesystem of a drive "
						"or floppy of yours is different from the content that is saved in the save file you are currently trying to load.\n\n"
						"Do you want to keep the content on the disk or overwrite it with the content in the save file?\n\n"
						"FileSystem UUID:'%s'"), *Name)))
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1)
				.HAlign(HAlign_Right)[
					SNew(SButton)
					.Text(FText::FromString("Keep changes on disk"))
					.OnClicked_Lambda([&KeepDisk, &Window]() {
						KeepDisk = 1;
						Window->RequestDestroyWindow();
						return FReply::Handled();
					})]
				+SHorizontalBox::Slot()
				.AutoWidth()[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Overwrite with Save")))
					.OnClicked_Lambda([&KeepDisk, &Window]() {
						KeepDisk = 0;
						Window->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
			]
		];
	FSlateApplication::Get().AddModalWindow(Window, GEngine->GetGameViewportWidget());
	return KeepDisk;
}

#define CheckKeepDisk(Condition) \
	if (bIsLoading && KeepDisk == -1) { \
		if (Condition) KeepDisk = AskForDiskOrSave(Name); \
		if (KeepDisk == 1) return; \
	}

void AFINFileSystemState::SerializePath(CodersFileSystem::SRef<CodersFileSystem::Device> SerializeDevice, FStructuredArchive::FRecord Record, CodersFileSystem::Path Path, FString Name, int& KeepDisk) {
	std::unordered_set<std::string> childs;
	std::unordered_set<std::string>::iterator childIterator;
	int ChildNodeNum;
	bool bIsSaving = Record.GetUnderlyingArchive().IsSaving();
	bool bIsLoading = Record.GetUnderlyingArchive().IsLoading();

	if (bIsSaving) {
		childs = SerializeDevice->childs(Path);
		childIterator = childs.begin();
		ChildNodeNum = childs.size();
	}
	FStructuredArchive::FArray ChildNodes = Record.EnterArray(SA_FIELD_NAME(TEXT("ChildNodes")), ChildNodeNum);
	std::unordered_set<std::string> DiskChilds = SerializeDevice->childs(Path);
	CheckKeepDisk(DiskChilds.size() != ChildNodeNum);
	if (KeepDisk == 0) {
		for (std::string DiskChild : DiskChilds) {
			SerializeDevice->remove(Path / DiskChild, true);
		}
		DiskChilds.clear();
	}

	for (int i = 0; i < ChildNodeNum; ++i) {
		FStructuredArchive::FRecord Child = ChildNodes.EnterElement().EnterRecord();
		FString UChildName;
		if (Record.GetUnderlyingArchive().IsSaving()) {
			UChildName = UTF8_TO_TCHAR((childIterator++)->c_str());
		}
		Child.EnterField(SA_FIELD_NAME(TEXT("Name"))) << UChildName;
		std::string stdChildName = TCHAR_TO_UTF8(*UChildName);
		
		CheckKeepDisk(DiskChilds.find(stdChildName) == DiskChilds.end())
		if (DiskChilds.size() > 0 && KeepDisk == 0) {
			for (std::string DiskChild : DiskChilds) {
				SerializeDevice->remove(Path / DiskChild, true);
			}
			DiskChilds.clear();
		} else if (KeepDisk == -1) {
			DiskChilds.erase(stdChildName);
		}
		
		CodersFileSystem::SRef<CodersFileSystem::Node> ChildNode = SerializeDevice->get(Path / stdChildName);
		int Type = 0;
		if (dynamic_cast<CodersFileSystem::File*>(ChildNode.get())) Type = 1;
		if (dynamic_cast<CodersFileSystem::Directory*>(ChildNode.get())) Type = 2;
		Child.EnterField(SA_FIELD_NAME(TEXT("Type"))) << Type;
		if (Type == 1) {
			CheckKeepDisk(!CodersFileSystem::SRef<CodersFileSystem::File>(SerializeDevice->get(Path / stdChildName)).isValid())
			if (KeepDisk == 0) {
				SerializeDevice->remove(Path / stdChildName, true);
			}
			FStructuredArchive::FSlot Content = Child.EnterField(SA_FIELD_NAME(TEXT("FileContent")));
			if (Record.GetUnderlyingArchive().IsLoading()) {
				std::string diskData;
				if (KeepDisk == -1) diskData = CodersFileSystem::FileStream::readAll(SerializeDevice->open(Path / stdChildName, CodersFileSystem::INPUT | CodersFileSystem::BINARY));
				FString Data;
				Content << Data;
				FTCHARToUTF8 Convert(*Data, Data.Len());
				std::string stdData = std::string(Convert.Get(), Convert.Length());
				CheckKeepDisk(diskData != stdData)
				if (KeepDisk == 0) {
					CodersFileSystem::SRef<CodersFileSystem::FileStream> Stream = SerializeDevice->open(Path / stdChildName, CodersFileSystem::OUTPUT | CodersFileSystem::TRUNC | CodersFileSystem::BINARY);
					Stream->write(stdData);
					Stream->close();
				}
            } else if (Record.GetUnderlyingArchive().IsSaving()) {
            	CodersFileSystem::SRef<CodersFileSystem::FileStream> Stream = SerializeDevice->open(Path / stdChildName, CodersFileSystem::INPUT | CodersFileSystem::BINARY);
            	std::string RawData = CodersFileSystem::FileStream::readAll(Stream);
            	Stream->close();
            	FUTF8ToTCHAR Convert(RawData.c_str(), RawData.length());
            	FString Data(Convert.Length(), Convert.Get());
            	Content << Data;
            }
		} else if (Type == 2) {
			CheckKeepDisk(!CodersFileSystem::SRef<CodersFileSystem::Directory>(SerializeDevice->get(Path / stdChildName)).isValid())
			if (KeepDisk == 0) {
				SerializeDevice->remove(Path / stdChildName, true);
			}
			SerializeDevice->createDir(Path / stdChildName);
			SerializePath(SerializeDevice, Child, Path / stdChildName, Name, KeepDisk);
			if (KeepDisk == 1) return;
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
		//SerializeDevice->remove("/", true);
	}
	
	FStructuredArchive::FSlot RootNode = Record.EnterField(SA_FIELD_NAME(TEXT("RootNode")));
	if (!bUseOldSerialization) {
		int KeepDisk = -1;
		SerializePath(SerializeDevice, RootNode.EnterRecord(), "/", ID.ToString(), KeepDisk);
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
			std::unordered_set<std::string> nodes = Device->childs("");
			for (std::string newNode : nodes) {
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
