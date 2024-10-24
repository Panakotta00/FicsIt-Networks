#include "FicsItFileSystem.h"

#define LOCTEXT_NAMESPACE "FFicsItFileSystemModule"

namespace CodersFileSystem {
	void CopyPath(TSharedRef<Device> FromDevice, TSharedRef<Device> ToDevice, Path Path) {
		for (std::string Child : FromDevice->children(Path)) {
			CodersFileSystem::Path ChildPath = Path / Child;
			TOptional<FileType> type = FromDevice->fileType(ChildPath);
			if (!type.IsSet()) continue;
			if (type == File_Regular) {
				TSharedPtr<FileStream> InputStream = FromDevice->open(ChildPath, FileMode::INPUT | FileMode::BINARY);
				TSharedPtr<FileStream> OutputStream = ToDevice->open(ChildPath, FileMode::OUTPUT | FileMode::BINARY);
				if (!InputStream.IsValid() || OutputStream.IsValid()) continue;
				OutputStream->write(FileStream::readAll(InputStream.ToSharedRef()));
				OutputStream->close();
				InputStream->close();
			} else if (type == File_Directory) {
				ToDevice->createDir(ChildPath);
				CopyPath(FromDevice, ToDevice, ChildPath);
			}
		}
	}

	void DeleteEntries(TSharedRef<Device> Device) {
		for (std::string Child : Device->children("/")) {
			Device->remove(Path(Child), true);
		}
	}
}

DEFINE_LOG_CATEGORY(LogFicsItFileSystem)

void FFicsItFileSystemModule::StartupModule() {}

void FFicsItFileSystemModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FFicsItFileSystemModule, FicsItFileSystem)