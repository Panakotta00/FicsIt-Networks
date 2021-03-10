#include "DevDevice.h"

FFINKernelFSDevDevice::FFINKernelFSDevDevice() {
	Serial = new FFINKernelFSSerial(FileSystem::ListenerListRef(listeners, ""));
}

FileSystem::SRef<FileSystem::FileStream> FFINKernelFSDevDevice::open(FileSystem::Path path, FileSystem::FileMode mode) {
	path.absolute = false;
	if (path == "serial") {
		return Serial->open(mode);
	}
	return nullptr;
}

FileSystem::SRef<FileSystem::Node> FFINKernelFSDevDevice::get(FileSystem::Path path) {
	try {
		return new FileSystem::DeviceNode(Devices.at(path.str()));
	} catch (...) {
		return nullptr;
	}
}

bool FFINKernelFSDevDevice::remove(FileSystem::Path path, bool recursive) {
	return false;
}

FileSystem::SRef<FileSystem::Directory> FFINKernelFSDevDevice::createDir(FileSystem::Path, bool tree) {
	return nullptr;
}

bool FFINKernelFSDevDevice::rename(FileSystem::Path path, const FileSystem::NodeName& name) {
	return false;
}

std::unordered_set<FileSystem::NodeName> FFINKernelFSDevDevice::childs(FileSystem::Path path) {
	std::unordered_set<FileSystem::NodeName> list;
	for (auto device : Devices) {
		list.insert(device.first);
	}
	list.insert("serial");
	return list;
}

bool FFINKernelFSDevDevice::addDevice(FileSystem::SRef<FileSystem::Device> device, const FileSystem::NodeName& name) {
	const auto dev = Devices.find(name);
	if (dev != Devices.end() || name == "serial") return false;
	Devices[name] = device;
	return true;
}

bool FFINKernelFSDevDevice::removeDevice(FileSystem::SRef<FileSystem::Device> device) {
	for (auto d = Devices.begin(); d != Devices.end(); d++) {
		if (d->second == device) {
			Devices.erase(d);
			return true;
		}
	}
	return false;
}

std::unordered_map<FileSystem::NodeName, FileSystem::SRef<FileSystem::Device>> FFINKernelFSDevDevice::getDevices() const {
	return Devices;
}

void FFINKernelFSDevDevice::updateCapacity(std::int64_t capacity) {
	for (auto& device : Devices) {
		if (FileSystem::MemDevice* memDev = dynamic_cast<FileSystem::MemDevice*>(device.second.get())) {
			memDev->capacity = memDev->getUsed() + capacity;
		}
	}
}

void FFINKernelFSDevDevice::tickListeners() {
	for (auto& device : Devices) {
		if (FileSystem::DiskDevice* diskDev = dynamic_cast<FileSystem::DiskDevice*>(device.second.get())) {
			diskDev->tickWatcher();
		}
	}
}

FileSystem::SRef<FFINKernelFSSerial> FFINKernelFSDevDevice::getSerial() const {
	return Serial;
}
