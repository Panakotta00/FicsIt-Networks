#include "FicsItKernel/FicsItFS/FINFileSystemState_Legacy.h"

#include "App.h"
#include "FGInventoryComponent.h"
#include "FINComputerSubsystem.h"
#include "Engine/Engine.h"
#include "FileSystemSerializationInfo.h"
#include "Framework/Application/SlateApplication.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SViewport.h"

#define KEEP_CHANGES 1
#define OVERRIDE_CHANGES 0

AFINFileSystemState_Legacy::AFINFileSystemState_Legacy() {
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

int AskForDiskOrSave_Legacy(FString Name) {
	switch (AFINComputerSubsystem::GetFSAlways()) {
	case FIN_FS_AlwaysOverride:
		return OVERRIDE_CHANGES;
	case FIN_FS_AlwaysKeep:
		return KEEP_CHANGES;
	default: ;
	}
	
	int KeepDisk = 0;
	bool bDontAskAgain = false;
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
				.AutoWidth()
				.HAlign(HAlign_Left)[
					SNew(SButton)
					.Text(FText::FromString("Keep changes on disk"))
					.OnClicked_Lambda([&KeepDisk, &Window]() {
						KeepDisk = KEEP_CHANGES;
						Window->RequestDestroyWindow();
						return FReply::Handled();
					})]
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)[
					SNew(SButton)
					.Text(FText::FromString(TEXT("Overwrite with Save")))
					.OnClicked_Lambda([&KeepDisk, &Window]() {
						KeepDisk = OVERRIDE_CHANGES;
						Window->RequestDestroyWindow();
						return FReply::Handled();
					})
				]
				+SHorizontalBox::Slot()
				.FillWidth(1)
				.HAlign(HAlign_Right)[
					SNew(SCheckBox)
					.Content()[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Don't ask again")))
					]
					.IsChecked_Lambda([&]() {
						return bDontAskAgain ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
					.OnCheckStateChanged_Lambda([&](ECheckBoxState State) {
						bDontAskAgain = State == ECheckBoxState::Checked;
					})
				]
			]
		];
	FSlateApplication::Get().AddModalWindow(Window, GEngine->GetGameViewportWidget());

	if (bDontAskAgain) {
		switch (KeepDisk) {
		case KEEP_CHANGES:
			AFINComputerSubsystem::SetFSAlways(FIN_FS_AlwaysKeep);
			break;
		case OVERRIDE_CHANGES:
			AFINComputerSubsystem::SetFSAlways(FIN_FS_AlwaysOverride);
			break;
		default: ;
		}
	}
	
	return KeepDisk;
}

#define CheckKeepDisk(Condition) \
	if (bIsLoading && KeepDisk == -1) { \
		if (Condition) KeepDisk = AskForDiskOrSave_Legacy(Name); \
		if (KeepDisk == 1) return; \
	}

void AFINFileSystemState_Legacy::SerializePath(TSharedRef<CodersFileSystem::Device> SerializeDevice, FStructuredArchive::FRecord Record, CodersFileSystem::Path Path, FString Name, int& KeepDisk) {
	std::unordered_set<std::string> childs;
	std::unordered_set<std::string>::iterator childIterator;
	int ChildNodeNum;
	bool bIsSaving = Record.GetUnderlyingArchive().IsSaving();
	bool bIsLoading = Record.GetUnderlyingArchive().IsLoading();

	if (bIsSaving) {
		childs = SerializeDevice->children(Path);
		childIterator = childs.begin();
		ChildNodeNum = childs.size();
	}
	FStructuredArchive::FArray ChildNodes = Record.EnterArray(SA_FIELD_NAME(TEXT("ChildNodes")), ChildNodeNum);
	std::unordered_set<std::string> DiskChilds = SerializeDevice->children(Path);
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

		CodersFileSystem::FileType ChildType = *SerializeDevice->fileType(Path / stdChildName);
		int Type = 0;
		if (ChildType == CodersFileSystem::File_Regular) Type = 1;
		if (ChildType == CodersFileSystem::File_Directory) Type = 2;
		Child.EnterField(SA_FIELD_NAME(TEXT("Type"))) << Type;
		TOptional<CodersFileSystem::FileType> existingType = SerializeDevice->fileType(Path / stdChildName);
		if (Type == 1) {
			CheckKeepDisk(!existingType.IsSet() || *existingType != CodersFileSystem::File_Regular)
			if (KeepDisk == 0) {
				SerializeDevice->remove(Path / stdChildName, true);
			}
			FStructuredArchive::FSlot Content = Child.EnterField(SA_FIELD_NAME(TEXT("FileContent")));
			if (Record.GetUnderlyingArchive().IsLoading()) {
				std::string diskData;
				if (KeepDisk == -1) diskData = CodersFileSystem::FileStream::readAll(SerializeDevice->open(Path / stdChildName, CodersFileSystem::INPUT | CodersFileSystem::BINARY).ToSharedRef());
				std::string stdData;
				if (bUsePreBinarySupportSerialization) {
					FString Data;
					Content << Data;
					FTCHARToUTF8 Conv(Data);
					stdData = std::string(Conv.Get(), Conv.Length());
				} else {
					TArray<uint8> Data;
					Content << Data;
					stdData = std::string((char*)Data.GetData(), Data.Num());
				}
				CheckKeepDisk(diskData != stdData)
				if (KeepDisk == 0) {
					TSharedPtr<CodersFileSystem::FileStream> Stream = SerializeDevice->open(Path / stdChildName, CodersFileSystem::OUTPUT | CodersFileSystem::TRUNC | CodersFileSystem::BINARY);
					Stream->write(stdData);
					Stream->close();
				}
            } else if (Record.GetUnderlyingArchive().IsSaving()) {
            	TSharedPtr<CodersFileSystem::FileStream> Stream = SerializeDevice->open(Path / stdChildName, CodersFileSystem::INPUT | CodersFileSystem::BINARY);
            	std::string RawData = CodersFileSystem::FileStream::readAll(Stream.ToSharedRef());
            	if (bUsePreBinarySupportSerialization) {
            		FUTF8ToTCHAR Conv(RawData.c_str(), RawData.length());
            		FString Data(Conv.Length(), Conv.Get());
            		Content << Data;
            	} else {
            		Stream->close();
            		TArray<uint8> Data((uint8*)RawData.c_str(), RawData.length());
            		Content << Data;
            	}
            }
		} else if (Type == 2) {
			CheckKeepDisk(!existingType.IsSet() || *existingType != CodersFileSystem::File_Directory)
			if (KeepDisk == 0) {
				SerializeDevice->remove(Path / stdChildName, true);
			}
			SerializeDevice->createDir(Path / stdChildName);
			SerializePath(SerializeDevice, Child, Path / stdChildName, Name, KeepDisk);
			if (KeepDisk == 1) return;
		}
	}
}

void AFINFileSystemState_Legacy::Serialize(FStructuredArchive::FRecord Record) {
	Super::Serialize(Record);

	if (!Record.GetUnderlyingArchive().IsSaveGame()) return;

	int32 OldCapacity = Capacity;
	Capacity = 0;
	TSharedPtr<CodersFileSystem::Device> SerializeDevice = GetDevice(false, true);
	Capacity = OldCapacity;

	if (Record.GetUnderlyingArchive().IsLoading()) {
		//SerializeDevice->remove("/", true);
	}
	
	FStructuredArchive::FSlot RootNode = Record.EnterField(SA_FIELD_NAME(TEXT("RootNode")));
	if (!bUseOldSerialization) {
		int KeepDisk = -1;
		SerializePath(SerializeDevice.ToSharedRef(), RootNode.EnterRecord(), "/", ID.ToString(), KeepDisk);
	} else {
		bUseOldSerialization = false;
	}
}

void AFINFileSystemState_Legacy::PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	bUseOldSerialization = gameVersion < 150216;
	//bUsePreBinarySupportSerialization = gameVersion < 264901;
}

bool AFINFileSystemState_Legacy::ShouldSave_Implementation() const {
	return true;
}

TSharedPtr<CodersFileSystem::Device> AFINFileSystemState_Legacy::GetDevice(bool bInForceUpdate, bool bInForceCreate) {
	if (!HasAuthority()) return nullptr;
	
	if (!IdCreated) {
		IdCreated = true;
		ID = FGuid::NewGuid();
	}

	TSharedPtr<CodersFileSystem::Device> NewDevice = Device;
	if (!Device.IsValid() || bInForceCreate || bInForceUpdate) {
		FString fsp;
		// TODO: Get UFGSaveSystem::GetSaveDirectoryPath() working
		if(fsp.IsEmpty()) {
			fsp = FPaths::Combine( FPlatformProcess::UserSettingsDir(), FApp::GetProjectName(), TEXT( "Saved/" ) TEXT( "SaveGames/" ) );
		}

		// get root fs path
		std::filesystem::path root = std::filesystem::absolute(*fsp);
		root = root / std::filesystem::path(std::string("Computers")) / std::filesystem::path(TCHAR_TO_UTF8(*ID.ToString()));

		std::filesystem::create_directories(root);

		NewDevice = MakeShared<CodersFileSystem::DiskDevice>(root, Capacity);

		if (!Device.IsValid() || bInForceUpdate) Device = NewDevice;
	}
	return NewDevice;
}
