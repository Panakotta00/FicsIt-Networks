#include "FicsItFileSystem/Device.h"
#include "FicsItFileSystem/FileSystemRoot.h"
#include <iostream>
#include <cstdint>

using namespace std;
namespace fs = std::filesystem;
namespace CodersFileSystem {
	ByteCountedDevice::ByteCountedDeviceListener::ByteCountedDeviceListener(ByteCountedDevice* root) : root(root) {}

	void CodersFileSystem::ByteCountedDevice::ByteCountedDeviceListener::onMounted(Path path, TSharedRef<Device> device) {
		if (root->listenerMask & 0b000001) root->usedValid = false;
	}

	void CodersFileSystem::ByteCountedDevice::ByteCountedDeviceListener::onUnmounted(Path path, TSharedRef<Device> device) {
		if (root->listenerMask & 0b000010) root->usedValid = false;
	}

	void CodersFileSystem::ByteCountedDevice::ByteCountedDeviceListener::onNodeAdded(Path path, NodeType type) {
		if (root->listenerMask & 0b000100) root->usedValid = false;
	}

	void CodersFileSystem::ByteCountedDevice::ByteCountedDeviceListener::onNodeRemoved(Path path, NodeType type) {
		if (root->listenerMask & 0b001000) root->usedValid = false;
	}

	void CodersFileSystem::ByteCountedDevice::ByteCountedDeviceListener::onNodeChanged(Path path, NodeType type) {
		if (root->listenerMask & 0b010000) root->usedValid = false;
	}

	void CodersFileSystem::ByteCountedDevice::ByteCountedDeviceListener::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
		if (root->listenerMask & 0b100000) root->usedValid = false;
	}

	void Device::addListener(TWeakPtr<Listener> listener) {
		listeners.Add(listener);
	}

	void Device::removeListener(TWeakPtr<Listener> listener) {
		listeners.Remove(listener);
	}

	bool ByteCountedDevice::checkSizeFunc(long long size, bool addIfAble) {
		if (capacity < 1) return true;
		size_t newUsed = getUsed();
		if (newUsed + size > capacity) return false;
		used += size;
		return true;
	}

	ByteCountedDevice::ByteCountedDevice(size_t capacity) : capacity(capacity) {
		addListener(byteCountedDeviceListener = MakeShared<ByteCountedDeviceListener>(this));
		checkSize = std::bind(&ByteCountedDevice::checkSizeFunc, this, std::placeholders::_1, std::placeholders::_2);
	}

	size_t ByteCountedDevice::getUsed() {
		if (capacity < 1) return 0;
		if (!usedValid) {
			used = this->getSize();
			usedValid = true;
		}
		return used;
	}

	size_t getSizeFromPath(fs::path e) {
		uint64_t count = 0;
		if (fs::is_directory(e)) {
			count += e.filename().string().length();
			for (auto& i : fs::directory_iterator(e)) {
				count += getSizeFromPath(i);
			}
		} else if (fs::is_regular_file(e)) {
			count += e.filename().string().length();
			count += fs::file_size(e);
		}
		return count;
	}

	size_t DiskDevice::getSize() const {
		return getSizeFromPath(realPath);
	}

	DiskDevice::DiskDevice(fs::path realPath, size_t capacity) : ByteCountedDevice(capacity), realPath(realPath), watcher(realPath,
		[&](int eventType, auto node, auto to, auto from) {
			switch (eventType) {
			case 0:
				listeners.onNodeAdded(to, node);
				break;
			case 1:
				listeners.onNodeRemoved(to, node);
				break;
			case 2:
				listeners.onNodeChanged(to, node);
				break;
			case 3:
				listeners.onNodeRenamed(to, from, node);
				break;
			}
		}) {
		getUsed();
	}

	TSharedPtr<FileStream> DiskDevice::open(Path path, FileMode mode) {
		std::filesystem::path spath = realPath / path.relative().str();
		if (fs::exists(spath) && !fs::is_regular_file(spath)) return nullptr;
		else if (!fs::is_directory(spath / "..")) return nullptr;
		return MakeShared<DiskFileStream>(spath, mode, checkSize);
	}

	bool DiskDevice::createDir(Path path, bool createTree) {
		std::filesystem::path spath = realPath / path.relative().str();
		if (fs::exists(spath)) {
			if (fs::is_directory(spath)) return true;
			else return false;
		}
		
		if (createTree) {
			fs::create_directories(spath);
		} else if (fs::is_directory(spath / "..")) {
			fs::create_directory(spath);
		} else return false;
		tickListeners();
		return true;
	}

	bool DiskDevice::remove(Path path, bool recursive) {
		if (path.isEmpty()) return false;
		std::filesystem::path spath = realPath / path.relative().str();
		try {
			if (recursive) return fs::remove_all(spath) > 0;
			else return fs::remove(spath);
			tickListeners();
		} catch (...) {
			return false;
		}
	}

	bool DiskDevice::rename(Path path, const std::string& name) {
		if (path.isEmpty() || !Path::isNode(name)) return false;
		path = path.relative();
		std::filesystem::path spath = realPath / path.str();
		if (!fs::exists(spath) || fs::exists(realPath / (path / ".." / name).str()) || path.isRoot()) return false;
		fs::rename(spath, realPath / (path / ".." / name).str());
		tickListeners();
		return true;
	}

	unordered_set<std::string> DiskDevice::children(Path path) {
		std::unordered_set<std::string> childs;
		std::filesystem::path spath = realPath / path.relative().str();
		std::filesystem::path NewPath = spath;
		for (const auto& entry : fs::directory_iterator(NewPath)) {
			std::string childName = entry.path().filename().string();
			childs.insert(childName);
		}
		return childs;
	}

	TOptional<FileType> DiskDevice::fileType(Path path) {
		auto fspath = realPath / path.relative().str();
		if (std::filesystem::is_directory(fspath)) {
			return File_Directory;
		} else if (std::filesystem::is_regular_file(fspath)) {
			return File_Regular;
		}
		return {};
	}

	void DiskDevice::tickListeners() {
		watcher.tick();
	}

	std::filesystem::path DiskDevice::getRealPath() const {
		return realPath;
	}
}