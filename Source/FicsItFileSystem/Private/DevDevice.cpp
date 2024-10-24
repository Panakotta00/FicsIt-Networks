#include "DevDevice.h"

FFINKernelFSDevDevice::FFINKernelFSDevDevice() {}

TSharedPtr<CodersFileSystem::FileStream> FFINKernelFSDevDevice::open(CodersFileSystem::Path path, CodersFileSystem::FileMode mode) {
	return nullptr;
}

bool FFINKernelFSDevDevice::remove(CodersFileSystem::Path path, bool recursive) {
	return false;
}

bool FFINKernelFSDevDevice::createDir(CodersFileSystem::Path, bool tree) {
	return false;
}

bool FFINKernelFSDevDevice::rename(CodersFileSystem::Path path, const std::string& name) {
	return false;
}

std::unordered_set<std::string> FFINKernelFSDevDevice::children(CodersFileSystem::Path path) {
	std::unordered_set<std::string> list;
	for (auto device : Devices) {
		list.insert(device.first);
	}
	return list;
}

TSharedPtr<CodersFileSystem::Device> FFINKernelFSDevDevice::getDevice(CodersFileSystem::Path path) {
	if (!path.isSingle()) return nullptr;
	auto device = Devices.find(path);
	if (device != Devices.end()) {
		return device->second;
	}
	return nullptr;
}

TOptional<CodersFileSystem::FileType> FFINKernelFSDevDevice::fileType(CodersFileSystem::Path path) {
	if (!path.isSingle()) return {};
	auto device = Devices.find(path);
	if (device != Devices.end()) {
		return CodersFileSystem::File_Device;
	}
	return {};
}

bool FFINKernelFSDevDevice::addDevice(TSharedRef<CodersFileSystem::Device> device, const std::string& name) {
	const auto dev = Devices.find(name);
	if (dev != Devices.end()) return false;
	Devices.emplace(name, device);
	return true;
}

bool FFINKernelFSDevDevice::removeDevice(TSharedRef<CodersFileSystem::Device> device) {
	for (auto d = Devices.begin(); d != Devices.end(); d++) {
		if (d->second == device) {
			Devices.erase(d);
			return true;
		}
	}
	return false;
}

std::unordered_map<std::string, TSharedRef<CodersFileSystem::Device>> FFINKernelFSDevDevice::getDevices() const {
	return Devices;
}

void FFINKernelFSDevDevice::updateCapacity(std::int64_t capacity) {
	for (const auto& [_, device] : Devices) {
		// TODO: Get Capacity
	}
}

void FFINKernelFSDevDevice::tickListeners() {
	for (const auto& [_, device] : Devices) {
		device->tickListeners();
	}
}
