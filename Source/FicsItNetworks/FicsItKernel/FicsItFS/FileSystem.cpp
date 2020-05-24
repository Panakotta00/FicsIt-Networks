#include "FileSystem.h"

#include "CoreMinimal.h"
#include "FileSystemSerializationInfo.h"
#include "Library/NodeName.h"
#include "util/Logging.h"

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
			std::int64_t memoryUsage = 0;
			for (auto m : mounts) {
				FileSystem::SRef<FileSystem::MemDevice> tmpDev = m.second.first;
				if (tmpDev.isValid()) {
					if (recalc) memoryUsage += tmpDev->getSize();
					else memoryUsage += tmpDev->getUsed();
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

		FileSystem::Path Root::getMountPoint(FileSystem::SRef<DevDevice> device) {
			for (auto& mount : mounts) {
				if (device == mount.second.first) return mount.first;
			}
			return "";
		}

		std::string Root::persistPath(FileSystem::Path path) {
			FileSystem::Path pending;
			FileSystem::SRef<FileSystem::Device> dev = getDevice(path, pending);
			for (auto& device : getDevDevice()->getDevices()) {
				if (device.second == dev) {
					FileSystem::NodeName name = device.first;
					return name + ":" + pending.str();
				}
			}
			throw std::invalid_argument("Unable to persist path");
		}

		FileSystem::Path Root::unpersistPath(std::string path) {
			size_t pos = path.find(':');
			FileSystem::NodeName name = path.substr(0, pos);
			FileSystem::Path pending = path.substr(pos+1);
			for (auto& device : getDevDevice()->getDevices()) {
				if (device.first == name) {
					for (auto& mount : mounts) {
						if (mount.second.first == device.second) {
							return mount.first / pending;
						}
					}
				}
			}
			throw std::invalid_argument("Unable to unpersist path");
		}

		void Root::Serialize(FArchive& Ar, FFileSystemSerializationInfo& info) {
			if (Ar.IsSaving() && getDevDevice()) {
				// serialize mount points
				for (auto mount : mounts) {
					for (auto device : getDevDevice()->getDevices()) {
						if (mount.second.first == device.second) {
							info.Mounts.Add(device.first.c_str(), mount.first.str().c_str());
							break;
						}
					}
				}

				// serialize tempfs
				for (std::pair<const FileSystem::NodeName, FileSystem::SRef<FileSystem::Device>> dev : getDevDevice()->getDevices()) {
					if (!dynamic_cast<FileSystem::MemDevice*>(dev.second.get())) continue;
					FFileSystemNode node = FFileSystemNode().Serialize(dev.second, "/");
					node.NodeType = 3;
					info.Devices.Add(dev.first.c_str(), node);
				}
			}
			Ar << info.Mounts;
			Ar << info.Devices;
		}

		void Root::PostLoad(const FFileSystemSerializationInfo& info) {
			FileSystem::SRef<DevDevice> devDev = getDevDevice();
			if (!devDev.isValid()) return;
			
			// deserialize/generate tmpfs
			for (TPair<FString, FFileSystemNode> device : info.Devices) {
				std::string deviceName = TCHAR_TO_UTF8(*device.Key);
				if (device.Value.NodeType == 3) {
					FileSystem::SRef<FileSystem::Device> dev = new FileSystem::MemDevice();
					if (!devDev->addDevice(dev, deviceName)) {
						SML::Logging::error(("Unable to unpersist tmpfs '" + deviceName + "'").c_str());
						continue;
					}
					device.Value.Deserialize(dev, deviceName);
				}
			}
			
			// lodd mounts
			for (TPair<FString, FString> mount : info.Mounts) {
				for (auto device : devDev->getDevices()) {
					if (FString(device.first.c_str()) == mount.Key) {
						this->mount(device.second, TCHAR_TO_UTF8(*mount.Value));
						break;
					}
				}
			}
		}
	}
}
