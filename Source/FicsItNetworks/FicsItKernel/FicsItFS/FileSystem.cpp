#include "FileSystem.h"

namespace FicsItKernel {
	namespace FicsItFS {
		bool Root::remove(FileSystem::Path path, bool recursive) {
			if (path.startsWith("/dev")) return false;
			return FileSystemRoot::remove(path, recursive);
		}

		bool Root::rename(FileSystem::Path path, const std::string & name) {
			if (path.startsWith("/dev")) return false;
			return FileSystemRoot::rename(path, name);
		}

		bool Root::mount(FileSystem::SRef<FileSystem::Device> device, FileSystem::Path path) {
			if (path.startsWith("/dev") || path.startsWith("/")) return false;
			return FileSystemRoot::mount(device, path);
		}

		bool Root::unmount(FileSystem::Path path) {
			if (path.startsWith("/dev") || path.startsWith("/")) return false;
			return FileSystemRoot::unmount(path);
		}

		std::int64_t Root::getMemoryUsage(bool recalc) {
			if (recalc) {
				memoryUsage = 0;
				for (auto m : mounts) {
					FileSystem::SRef<FileSystem::MemDevice> tmpDev = m.second.first;
					if (tmpDev.isValid()) memoryUsage += tmpDev->getUsed();
				}
			}
			return memoryUsage;
		}
	}
}