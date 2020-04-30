#include "WindowsFileWatcher.h"

#include "Engine.h"
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

#include "Path.h"
#include "Listener.h"

#include "CoreMinimal.h"

namespace fs = std::filesystem;

namespace FileSystem {
	struct DiskDeviceWatcher {
		HANDLE watcher;
		OVERLAPPED ovl;
		FILE_NOTIFY_INFORMATION info[16];
	};

	WindowsFileWatcher::WindowsFileWatcher(const std::filesystem::path& path, std::function<void(int, NodeType, Path, Path)> event) : eventFunc(event), realPath(path) {
		watcherInfo = new DiskDeviceWatcher();
		watcherInfo->watcher = ::CreateFile(path.wstring().c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);
		watcherInfo->ovl = {0};
		watcherInfo->ovl.hEvent = CreateEvent(NULL, true, false, NULL);
		memset(&watcherInfo->info, 0, sizeof(watcherInfo->info));
		ReadDirectoryChangesW(watcherInfo->watcher, &watcherInfo->info, sizeof(watcherInfo->info), true, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &watcherInfo->ovl, NULL);
	}

	WindowsFileWatcher::~WindowsFileWatcher() {
		delete watcherInfo;
	}

	void WindowsFileWatcher::tick() {
		DWORD status = WaitForSingleObject(watcherInfo->ovl.hEvent, 0);
		if (status != WAIT_OBJECT_0) return;

		FILE_NOTIFY_INFORMATION* current = &watcherInfo->info[0];
		std::wstring bufStr;
		while (current) {
			std::wstring fname = std::wstring((const wchar_t*)&current->FileName, current->FileNameLength);
			std::replace(fname.begin(), fname.end(), L'\\', L'/');
			Path path = fs::path(fname);
			bool isDir = fs::is_directory(realPath / path);
			NodeType type = (isDir) ? NT_Directory : NT_File;
			switch (current->Action) {
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
				eventFunc(3, type, path, fs::path(bufStr));
				break;
			case FILE_ACTION_RENAMED_OLD_NAME:
				bufStr = fname;
				break;
			}
			if (current->NextEntryOffset <= 0) break;
			auto old = current;
			current = (FILE_NOTIFY_INFORMATION*)((size_t)current + current->NextEntryOffset);
		}
		memset(&watcherInfo->info, 0, sizeof(watcherInfo->info));
		ReadDirectoryChangesW(watcherInfo->watcher, &watcherInfo->info, sizeof(watcherInfo->info), true, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE, NULL, &watcherInfo->ovl, NULL);
	}
}
