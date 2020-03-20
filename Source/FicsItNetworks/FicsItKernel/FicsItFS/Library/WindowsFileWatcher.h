#pragma once

#include <functional>

#include "FileSystem.h"

namespace FileSystem {
	enum NodeType;
	class Path;

	struct DiskDeviceWatcher;
	
	class WindowsFileWatcher {
	public:
		DiskDeviceWatcher* watcherInfo = nullptr;
		std::function<void(int, NodeType, Path, Path)> eventFunc;
		std::filesystem::path realPath;

		WindowsFileWatcher(const std::filesystem::path& path, std::function<void(int, NodeType, Path, Path)> eventFunc);
		~WindowsFileWatcher();
		void tick();
	};
}