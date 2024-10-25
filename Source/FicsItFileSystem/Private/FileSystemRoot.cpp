#include "FileSystemRoot.h"

#include "FicsItFileSystem.h"
#include "FileSystemException.h"

bool FFINKernelFSRoot::mount(TSharedRef<CodersFileSystem::Device> device, CodersFileSystem::Path path) {
	return FileSystemRoot::mount(device, path);
}

bool FFINKernelFSRoot::unmount(CodersFileSystem::Path path) {
	return FileSystemRoot::unmount(path);
}

bool FFINKernelFSRoot::unmount(TSharedRef<CodersFileSystem::Device> device) {
	CodersFileSystem::Path p;
	bool found = false;
	for (auto m : mounts) {
		if (m.second.first == device) {
			p = m.first;
			found = true;
			break;
		}
	}
	if (found) return unmount(p);
	return false;
}

int64 FFINKernelFSRoot::getMemoryUsage(bool recalc) {
	int64 memoryUsage = 0;
	for (auto m : mounts) {
		// TODO: Memory Usage
	}
	return memoryUsage;
}

CodersFileSystem::Path FFINKernelFSRoot::getMountPoint(TSharedRef<FFINKernelFSDevDevice> device) {
	for (auto& mount : mounts) {
		if (device == mount.second.first) return mount.first;
	}
	return "";
}

void FFINKernelFSRoot::Serialize(FStructuredArchive::FRecord Record, FFileSystemSerializationInfo& info) {
	/*if (Record.GetUnderlyingArchive().IsSaving() && getDevDevice()) {
		// serialize mount points
		for (auto mount : mounts) {
			for (auto device : getDevDevice()->getDevices()) {
				if (mount.second.first == device.second) {
					info.Mounts.Add(device.first.c_str(), mount.first.str().c_str());
					break;
				}
			}
		}

		// serialize temp-fs
		for (std::pair<const std::string, CodersFileSystem::SRef<CodersFileSystem::Device>> dev : getDevDevice()->getDevices()) {
			if (!dynamic_cast<CodersFileSystem::MemDevice*>(dev.second.get())) continue;
			FFileSystemNode node = FFileSystemNode().Serialize(dev.second, "/");
			node.NodeType = 3;
			info.Devices.Add(dev.first.c_str(), node);
		}
	}
	Record.EnterField(SA_FIELD_NAME(TEXT("Mounts"))) << info.Mounts;
	Record.EnterField(SA_FIELD_NAME(TEXT("Devices"))) << info.Devices;*/
}

void FFINKernelFSRoot::PostLoad(const FFileSystemSerializationInfo& info) {
	/*CodersFileSystem::SRef<FFINKernelFSDevDevice> devDev = getDevDevice();
	if (!devDev.isValid()) return;
	
	// deserialize/generate tmpfs
	for (TPair<FString, FFileSystemNode> device : info.Devices) {
		std::string deviceName = TCHAR_TO_UTF8(*device.Key);
		if (device.Value.NodeType == 3) {
			const CodersFileSystem::SRef<CodersFileSystem::Device> dev = new CodersFileSystem::MemDevice();
			if (!devDev->addDevice(dev, deviceName)) {
				UE_LOG(LogFicsItFileSystem, Error, TEXT("Unable to unpersist tmpfs '%s'"), *FString(deviceName.c_str()));
				continue;
			}
			device.Value.Deserialize(dev, deviceName);
		}
	}
	
	// load mounts
	for (TPair<FString, FString> mount : info.Mounts) {
		for (auto device : devDev->getDevices()) {
			if (FString(device.first.c_str()) == mount.Key) {
				this->mount(device.second, TCHAR_TO_UTF8(*mount.Value));
				break;
			}
		}
	}*/
}
