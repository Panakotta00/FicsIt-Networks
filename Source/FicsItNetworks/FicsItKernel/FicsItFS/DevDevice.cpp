#include "DevDevice.h"

using namespace FicsItKernel;
using namespace FicsItKernel::FicsItFS;

DevDevice::DevDevice() {
	stdio = new FileSystem::MemFile(FileSystem::ListenerListRef(listeners, ""), nullptr);
}

FileSystem::SRef<FileSystem::FileStream> DevDevice::open(FileSystem::Path path, FileSystem::FileMode mode) {
	path.absolute = false;
	if (path == "stdio") {
		return stdio->open(mode);
	}
	return nullptr;
}

FileSystem::SRef<FileSystem::Node> DevDevice::get(FileSystem::Path path) {
	try {
		return devices.at(path);
	} catch (...) {
		return nullptr;
	}
}

bool DevDevice::remove(FileSystem::Path path, bool recursive) {
	return false;
}

FileSystem::SRef<FileSystem::Directory> DevDevice::createDir(FileSystem::Path, bool tree) {
	return nullptr;
}

bool DevDevice::rename(FileSystem::Path path, const std::string & name) {
	return false;
}

std::unordered_set<std::string> DevDevice::childs(FileSystem::Path path) {
	std::unordered_set<std::string> list;
	for (auto device : devices) {
		list.insert(device.first);
	}
	return list;
}

bool DevDevice::addDevice(FileSystem::SRef<FileSystem::Device> device, const std::string & name) {
	auto dev = devices.find(name);
	if (dev != devices.end()) return false;
	devices[name] = device;
	return true;
}

bool DevDevice::removeDevice(FileSystem::SRef<FileSystem::Device> device) {
	for (auto d = devices.begin(); d != devices.end(); d++) {
		if (d->second == device) {
			devices.erase(d);
			return true;
		}
	}
	return false;
}
