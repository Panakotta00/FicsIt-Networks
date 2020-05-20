#include "FileSystem.h"

#include "CoreMinimal.h"
#include "FileSystemSerializationInfo.h"

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

		TSharedPtr<FFileSystemNode> serializeNodePath(FileSystem::SRef<FileSystem::Device> device, const FileSystem::Path& path) {
			FileSystem::SRef<FileSystem::Node> node = device->get(path);
			TSharedPtr<FFileSystemNode> result = MakeShareable(new FFileSystemNode());
			if (FileSystem::SRef<FileSystem::File> file = node) {
				result->NodeType = 0;
				FileSystem::SRef<FileSystem::FileStream> stream = file->open(FileSystem::INPUT);
				result->Data = stream->readAll().c_str();
			} else if (FileSystem::SRef<FileSystem::Directory> dir = node) {
				result->NodeType = 1;
				for (FileSystem::NodeName child : dir->getChilds()) {
					result->ChildNodes[child.c_str()] = serializeNodePath(device, path / child);
				}
			}
			return result;
		} 

		void Root::Serialize(FArchive& Ar, FFileSystemSerializationInfo& info) {
			// serialize mount points
			if (Ar.IsSaving() && getDevDevice()) for (auto mount : mounts) {
				for (auto device : getDevDevice()->getDevices()) {
					if (mount.second.first == device.second) {
						info.Mounts.Add(device.first.c_str(), mount.first.str().c_str());
						break;
					}
				}
			}
			Ar << info.Mounts;

			// serialize filesystem devices
			if (Ar.IsSaving() && getDevDevice()) for (auto device : getDevDevice()->getDevices()) {
				FFileSystemNode fsn;
				if (dynamic_cast<FileSystem::MemDevice*>(device.second.get())) fsn.NodeType = 3;
				if (dynamic_cast<FileSystem::DiskDevice*>(device.second.get())) fsn.NodeType = 2;
				if (fsn.NodeType == 2 || fsn.NodeType == 3) {
					std::unordered_set<FileSystem::NodeName> nodes = device.second->childs("");
					for (FileSystem::NodeName node : nodes) {
						fsn.ChildNodes.Add(FString(node.c_str()), serializeNodePath(device.second, node));
					}
					info.FileSystemDevices.Add(FString(device.first.c_str()), fsn);
				}
			}
			Ar << info.FileSystemDevices;
		}

		FileSystem::SRef<FileSystem::Node> DeserializeFileSystemNode(const FFileSystemNodeIndex& node, FString name, FileSystem::SRef<FileSystem::Directory> parent) {
			std::string nodeName = TCHAR_TO_UTF8(*name);
			switch (node.Node->NodeType) {
			case 0: {
				FileSystem::SRef<FileSystem::File> file = parent->createFile(nodeName);
				FileSystem::SRef<FileSystem::FileStream> stream = file->open(FileSystem::OUTPUT | FileSystem::TRUNC);
				stream->write(TCHAR_TO_UTF8(*node.Node->Data));
				stream->flush();
				stream->close();
				return file;
			} case 1: {
				FileSystem::SRef<FileSystem::Directory> dir = parent->createSubdir(nodeName);
				for (TPair<FString, FFileSystemNodeIndex>& child : node.Node->ChildNodes) {
					DeserializeFileSystemNode(child.Value, child.Key, dir);
				}
				return dir;
			} default: {
				return nullptr;
			}
			}
		}

		void Root::PostLoad(const FFileSystemSerializationInfo& info) {
			// load devices
			for (TPair<FString, FFileSystemNode> deviceInfo : info.FileSystemDevices) {
				std::string deviceName = TCHAR_TO_UTF8(*deviceInfo.Key);
				if (deviceInfo.Value.NodeType == 2 || deviceInfo.Value.NodeType == 3) {
					// create tmpfs
					if (deviceInfo.Value.NodeType == 3) if (!getDevDevice()->addDevice(new FileSystem::MemDevice(), deviceName)) throw std::exception((std::string("Unable to create persisted tmpfs '") + deviceName + "'").c_str());
					// get the device we should deserialize
					FileSystem::SRef<FileSystem::Device> device = getDevDevice()->getDevices()[deviceName];
					if (!device.isValid()) throw std::exception(("unable to find device to unpersist '" + deviceName + "'").c_str());
					// delete previously existing contents
					if (deviceInfo.Value.NodeType == 2) {
						for (FileSystem::NodeName child : device->childs("/")) device->remove(child, true);
					}
					// deserialize children
					FileSystem::SRef<FileSystem::Directory> root = device->get("/");
					if (!root.isValid()) throw std::exception(("root of device '" + deviceName + "' can not be found").c_str());
					for (TPair<FString, FFileSystemNodeIndex>& child : deviceInfo.Value.ChildNodes) {
						DeserializeFileSystemNode(child.Value, child.Key, root);
					}
				}
			}
			
			// lodd mounts
			for (TPair<FString, FString> mount : info.Mounts) {
				for (auto device : getDevDevice()->getDevices()) {
					if (FString(device.first.c_str()) == mount.Key) {
						this->mount(device.second, TCHAR_TO_UTF8(*mount.Value));
						break;
					}
				}
			}
		}
	}
}
