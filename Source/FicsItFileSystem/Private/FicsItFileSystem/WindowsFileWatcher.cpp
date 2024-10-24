#include "FicsItFileSystem/WindowsFileWatcher.h"
#include "FicsItFileSystem/Listener.h"
#include "FicsItFileSystem/Path.h"

#if PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

namespace fs = std::filesystem;

namespace CodersFileSystem {
	struct DiskDeviceWatcher {
		HANDLE DirectoryHandle;
		OVERLAPPED OverlappedIO;
		uint8 Buffer[1024];
	};

	WindowsFileWatcher::WindowsFileWatcher(const std::filesystem::path& path, std::function<void(int, NodeType, Path, Path)> event) : eventFunc(event), realPath(path) {
		watcherInfo = new DiskDeviceWatcher();
		watcherInfo->DirectoryHandle = ::CreateFile(
			path.wstring().c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);
		ZeroMemory(&watcherInfo->OverlappedIO, sizeof(watcherInfo->OverlappedIO));
		watcherInfo->OverlappedIO.hEvent = CreateEvent(NULL, true, false, NULL);
		
		tryReadChanges();
	}

	WindowsFileWatcher::~WindowsFileWatcher() {
		CloseHandle(watcherInfo->OverlappedIO.hEvent);
		CloseHandle(watcherInfo->DirectoryHandle);
		delete watcherInfo;
	}

	void WindowsFileWatcher::tick() {
		DWORD status = WaitForSingleObject(watcherInfo->OverlappedIO.hEvent, 0);
		if (status != WAIT_OBJECT_0) return;

		::FILE_NOTIFY_INFORMATION* Event = (::FILE_NOTIFY_INFORMATION*)watcherInfo->Buffer;
		
		while (true) {
			handleChangeEvent(Event);
						
			if (Event->NextEntryOffset) {
				*((uint8**)&Event) += Event->NextEntryOffset;
			} else break;
		}
		tryReadChanges();
	}

	void WindowsFileWatcher::tryReadChanges() {
		memset(&watcherInfo->Buffer, 0, sizeof(watcherInfo->Buffer));
		ReadDirectoryChangesW(
			watcherInfo->DirectoryHandle,
			&watcherInfo->Buffer,
			sizeof(watcherInfo->Buffer),
			true,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
			NULL,
			&watcherInfo->OverlappedIO,
			NULL);
	}

	void WindowsFileWatcher::handleChangeEvent(FILE_NOTIFY_INFORMATION* changeEvent) {
		std::wstring fname = std::wstring((const wchar_t*)&changeEvent->FileName, changeEvent->FileNameLength);
		std::replace(fname.begin(), fname.end(), L'\\', L'/');
		Path path = fs::path(fname).string();
		bool isDir = fs::is_directory(realPath / path.str());
		NodeType type = (isDir) ? NT_Directory : NT_File;
		switch (changeEvent->Action) {
		case FILE_ACTION_ADDED:
			eventFunc(0, type, path, Path());
			break;
		case FILE_ACTION_REMOVED:
			eventFunc(1, type, path, Path());
			break;
		case FILE_ACTION_MODIFIED:
			eventFunc(2, type, path, Path());
			break;
		case FILE_ACTION_RENAMED_NEW_NAME:
			eventFunc(3, type, path, fs::path(oldNameBufString).string());
			break;
		case FILE_ACTION_RENAMED_OLD_NAME:
			oldNameBufString = fname;
			break;
		}
	}
}

#endif
