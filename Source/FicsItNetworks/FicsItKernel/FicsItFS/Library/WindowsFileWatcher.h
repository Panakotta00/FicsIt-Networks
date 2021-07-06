#pragma once

#include <functional>

#include "AkAcousticPortal.h"
#include "FileSystem.h"

namespace CodersFileSystem {
	enum NodeType;
	class Path;

	struct DiskDeviceWatcher;
	
	class WindowsFileWatcher {
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
		void handleChangeEvent(::FILE_NOTIFY_INFORMATION* changeEvent);
	};
}