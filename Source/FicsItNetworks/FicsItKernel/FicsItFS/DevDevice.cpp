#include "DevDevice.h"

FFINKernelFSDevDevice::FFINKernelFSDevDevice() {
	Serial = new FFINKernelFSSerial(CodersFileSystem::ListenerListRef(listeners, ""));
}

CodersFileSystem::SRef<CodersFileSystem::FileStream> FFINKernelFSDevDevice::open(CodersFileSystem::Path path, CodersFileSystem::FileMode mode) {
	path.absolute = false;
	if (path == "serial") {
		return Serial->open(mode);
	}
	return nullptr;
}

CodersFileSystem::SRef<CodersFileSystem::Node> FFINKernelFSDevDevice::get(CodersFileSystem::Path path) {
	try {
		return new CodersFileSystem::DeviceNode(Devices.at(path.str()));
	} catch (...) {
		return nullptr;
	}
}

bool FFINKernelFSDevDevice::remove(CodersFileSystem::Path path, bool recursive) {
	return false;
}

CodersFileSystem::SRef<CodersFileSystem::Directory> FFINKernelFSDevDevice::createDir(CodersFileSystem::Path, bool tree) {
	return nullptr;
}

bool FFINKernelFSDevDevice::rename(CodersFileSystem::Path path, const CodersFileSystem::NodeName& name) {
	return false;
}

std::unordered_set<CodersFileSystem::NodeName> FFINKernelFSDevDevice::childs(CodersFileSystem::Path path) {
	std::unordered_set<CodersFileSystem::NodeName> list;
	for (auto device : Devices) {
		list.insert(device.first);
	}
	list.insert("serial");
	return list;
}

bool FFINKernelFSDevDevice::addDevice(CodersFileSystem::SRef<CodersFileSystem::Device> device, const CodersFileSystem::NodeName& name) {
	const auto dev = Devices.find(name);
	if (dev != Devices.end() || name == "serial") return false;
	Devices[name] = device;
	return true;
}

bool FFINKernelFSDevDevice::removeDevice(CodersFileSystem::SRef<CodersFileSystem::Device> device) {
	for (auto d = Devices.begin(); d != Devices.end(); d++) {
		if (d->second == device) {
			Devices.erase(d);
			return true;
		}
	}
	return false;
}

std::unordered_map<CodersFileSystem::NodeName, CodersFileSystem::SRef<CodersFileSystem::Device>> FFINKernelFSDevDevice::getDevices() const {
	return Devices;
}

void FFINKernelFSDevDevice::updateCapacity(std::int64_t capacity) {
	for (auto& device : Devices) {
		if (CodersFileSystem::MemDevice* memDev = dynamic_cast<CodersFileSystem::MemDevice*>(device.second.get())) {
			memDev->capacity = memDev->getUsed() + capacity;
		}
	}
}

void FFINKernelFSDevDevice::tickListeners() {
	for (auto& device : Devices) {
		if (CodersFileSystem::DiskDevice* diskDev = dynamic_cast<CodersFileSystem::DiskDevice*>(device.second.get())) {
			diskDev->tickWatcher();
		}
	}
}

CodersFileSystem::SRef<FFINKernelFSSerial> FFINKernelFSDevDevice::getSerial() const {
	return Serial;
}
