#include "FileSystem.h"

namespace FicsItKernel {
	namespace FicsItFS {
		bool Root::mount(FileSystem::SRef<FileSystem::Device> device, FileSystem::Path path) {
			// if device is DevDevice, search for existing DevDevice in mounts & prevent mount if found
			if (dynamic_cast<DevDevice*>(device.get())) {
				if (getDevDevice().isValid()) return false;
			}

			return FileSystemRoot::mount(device, path);
		}

		bool Root::unmount(FileSystem::Path path) {
			// check if mount is DevDevice & if it is, prevent unmount
			auto mount = mounts.find(path);
			if (mount != mounts.end() && dynamic_cast<DevDevice*>(mount->second.first.get())) return false;

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

		FileSystem::WRef<DevDevice> Root::getDevDevice() {
			for (auto& mount : mounts) {
				if (DevDevice* device = dynamic_cast<DevDevice*>(mount.second.first.get())) return device;
			}
			return nullptr;
		}
	}
}