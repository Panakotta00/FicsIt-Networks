#include "FileSystemSerializationInfo.h"

#include "FicsItFileSystem.h"
#include "FileSystemException.h"

bool FFileSystemSerializationInfo::Serialize(FArchive& Ar) {
	Ar << Mounts;
	return true;
}

FArchive& operator<<(FArchive& Ar, FFileSystemSerializationInfo& Info) {
	Info.Serialize(Ar);
	return Ar;
}

namespace CodersFileSystem {
	#undef CheckKeepDisk
	#define CheckKeepDisk(Condition) \
	if (bIsLoading && KeepDisk == -1) { \
		if (Condition) KeepDisk = AskForDiskOrSave(Name); \
		if (KeepDisk == FIFS_KEEP_CHANGES) return; \
	}

	void SerializePath(TSharedRef<Device> SerializeDevice, FStructuredArchive::FRecord Record, Path Path, FString Name, int& KeepDisk, const TFunction<int(FString)>& AskForDiskOrSave) {
		std::unordered_set<std::string> childs;
		std::unordered_set<std::string>::iterator childIterator;
		int ChildNodeNum;
		bool bIsSaving = Record.GetUnderlyingArchive().IsSaving();
		bool bIsLoading = Record.GetUnderlyingArchive().IsLoading();

		if (bIsSaving) {
			childs = SerializeDevice->children(Path);
			childs.erase(".git");
			childIterator = childs.begin();
			ChildNodeNum = childs.size();
		}
		FStructuredArchive::FArray ChildNodes = Record.EnterArray(SA_FIELD_NAME(TEXT("ChildNodes")), ChildNodeNum);
		std::unordered_set<std::string> DiskChilds = SerializeDevice->children(Path);
		DiskChilds.erase(".git");
		CheckKeepDisk(DiskChilds.size() != ChildNodeNum);
		if (KeepDisk == FIFS_OVERRIDE_CHANGES) {
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
			if (DiskChilds.size() > 0 && KeepDisk == FIFS_OVERRIDE_CHANGES) {
				for (std::string DiskChild : DiskChilds) {
					SerializeDevice->remove(Path / DiskChild, true);
				}
				DiskChilds.clear();
			} else if (KeepDisk == -1) {
				DiskChilds.erase(stdChildName);
			}

			TOptional<FileType> ChildType = SerializeDevice->fileType(Path / stdChildName);
			int Type = 0;
			if (*ChildType == File_Regular) Type = 1;
			if (*ChildType == File_Directory) Type = 2;
			Child.EnterField(SA_FIELD_NAME(TEXT("Type"))) << Type;
			TOptional<FileType> existingType = SerializeDevice->fileType(Path / stdChildName);
			if (Type == 1) {
				CheckKeepDisk(!existingType.IsSet() || *existingType != File_Regular)
				if (KeepDisk == FIFS_OVERRIDE_CHANGES) {
					SerializeDevice->remove(Path / stdChildName, true);
				}
				FStructuredArchive::FSlot Content = Child.EnterField(SA_FIELD_NAME(TEXT("FileContent")));
				if (Record.GetUnderlyingArchive().IsLoading()) {
					std::string diskData;
					if (KeepDisk == -1) diskData = CodersFileSystem::FileStream::readAll(SerializeDevice->open(Path / stdChildName, CodersFileSystem::INPUT | CodersFileSystem::BINARY).ToSharedRef());

					TArray<uint8> Data;
					Content << Data;

					std::string stdData(reinterpret_cast<char*>(Data.GetData()), Data.Num());

					CheckKeepDisk(diskData != stdData)
					if (KeepDisk == FIFS_OVERRIDE_CHANGES) {
						TSharedRef<CodersFileSystem::FileStream> Stream = SerializeDevice->open(Path / stdChildName, CodersFileSystem::OUTPUT | CodersFileSystem::TRUNC | CodersFileSystem::BINARY).ToSharedRef();
						Stream->write(stdData);
						Stream->close();
					}
				} else if (Record.GetUnderlyingArchive().IsSaving()) {
					TSharedRef<CodersFileSystem::FileStream> Stream = SerializeDevice->open(Path / stdChildName, CodersFileSystem::INPUT | CodersFileSystem::BINARY).ToSharedRef();
					std::string RawData = CodersFileSystem::FileStream::readAll(Stream);
					Stream->close();

					TArray<uint8> Data((uint8*)RawData.c_str(), RawData.length());
					Content << Data;
				}
			} else if (Type == 2) {
				CheckKeepDisk(!existingType.IsSet() || *existingType != File_Directory)
				if (KeepDisk == FIFS_OVERRIDE_CHANGES) {
					SerializeDevice->remove(Path / stdChildName, true);
				}
				SerializeDevice->createDir(Path / stdChildName);
				SerializePath(SerializeDevice, Child, Path / stdChildName, Name, KeepDisk, AskForDiskOrSave);
				if (KeepDisk == FIFS_KEEP_CHANGES) return;
			}
		}
	}
}
