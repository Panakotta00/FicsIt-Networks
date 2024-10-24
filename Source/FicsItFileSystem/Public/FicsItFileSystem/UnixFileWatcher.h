#pragma once

#if PLATFORM_UNIX

#include <functional>

#include "FileSystem.h"
#include "Listener.h"

namespace CodersFileSystem {
	class FICSITFILESYSTEM_API FileWatcher {
	public:
		std::function<void(int, NodeType, Path, Path)> eventFunc;
		std::filesystem::path realPath;

		FileWatcher(const std::filesystem::path& path, std::function<void(int, NodeType, Path, Path)> eventFunc);
		~FileWatcher();
		void tick();
	};
}

#endif
