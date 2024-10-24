#pragma once

#if PLATFORM_WINDOWS

#include <functional>

#include "FileSystem.h"

struct _FILE_NOTIFY_INFORMATION;
typedef _FILE_NOTIFY_INFORMATION FILE_NOTIFY_INFORMATION;

namespace CodersFileSystem {
	enum NodeType;
	class Path;

	struct DiskDeviceWatcher;
	
	class FICSITFILESYSTEM_API WindowsFileWatcher {
	public:
		DiskDeviceWatcher* watcherInfo = nullptr;
		std::function<void(int, NodeType, Path, Path)> eventFunc;
		std::filesystem::path realPath;
		std::wstring oldNameBufString;

		WindowsFileWatcher(const std::filesystem::path& path, std::function<void(int, NodeType, Path, Path)> eventFunc);
		~WindowsFileWatcher();
		void tick();

	private:
		void tryReadChanges();
		void handleChangeEvent(struct ::FILE_NOTIFY_INFORMATION* changeEvent);
	};

	typedef WindowsFileWatcher FileWatcher;
}

#endif
