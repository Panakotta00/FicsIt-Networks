#include "FicsItKernel/FicsItFS/FINItemStateFileSystem.h"

#include "FINComputerSubsystem.h"
#include "FINFileSystemSubsystem.h"
#include "FicsItKernel/FicsItFS/FileSystemSerializationInfo.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "SViewport.h"

FIN_STRUCT_IMPLEMENT_INTERFACE(FFINItemStateFileSystem, FFINLabelContainerInterface)

#define KEEP_CHANGES 1
#define OVERRIDE_CHANGES 0

int AskForDiskOrSave(FString Name) {
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
		if (Condition) KeepDisk = AskForDiskOrSave(Name); \
		if (KeepDisk == 1) return; \
	}

void SerializePath(CodersFileSystem::SRef<CodersFileSystem::Device> SerializeDevice, FStructuredArchive::FRecord Record, CodersFileSystem::Path Path, FString Name, int& KeepDisk) {
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

				TArray<uint8> Data;
				Content << Data;

				std::string stdData(reinterpret_cast<char*>(Data.GetData()), Data.Num());

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

            	TArray<uint8> Data((uint8*)RawData.c_str(), RawData.length());
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

bool FFINItemStateFileSystem::Serialize(FStructuredArchive::FSlot Slot) {
	StaticStruct()->SerializeBin(Slot, this);

	if (!Slot.GetUnderlyingArchive().IsSaveGame()) return true;

	int32 OldCapacity = Capacity;
	Capacity = 0;
	CodersFileSystem::SRef<CodersFileSystem::Device> SerializeDevice = AFINFileSystemSubsystem::GetDevice(ID, false, true);
	Capacity = OldCapacity;

	auto Record = Slot.EnterRecord();
	FStructuredArchive::FSlot RootNode = Record.EnterField(SA_FIELD_NAME(TEXT("RootNode")));

	int KeepDisk = -1;
	SerializePath(SerializeDevice, RootNode.EnterRecord(), "/", ID.ToString(), KeepDisk);

	return true;
}
