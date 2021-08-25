#include "Device.h"

#include <iostream>

#include "FileSystemRoot.h"

using namespace std;
namespace fs = std::filesystem;
namespace CodersFileSystem {
	ByteCountedDevice::ByteCountedDeviceListener::ByteCountedDeviceListener(ByteCountedDevice* root) : root(root) {}

	void CodersFileSystem::ByteCountedDevice::ByteCountedDeviceListener::onMounted(Path path, SRef<Device> device) {
		if (root->listenerMask & 0b000001) root->usedValid = false;
	}

	void CodersFileSystem::ByteCountedDevice::ByteCountedDeviceListener::onUnmounted(Path path, SRef<Device> device) {
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

	void Device::addListener(WRef<Listener> listener) {
		listeners.insert(listener);
	}

	void Device::removeListener(WRef<Listener> listener) {
		listeners.erase(listener);
	}

	bool ByteCountedDevice::checkSizeFunc(long long size, bool addIfAble) {
		if (capacity < 1) return true;
		size_t newUsed = getUsed();
		if (newUsed + size > capacity) return false;
		used += size;
		return true;
	}

	ByteCountedDevice::ByteCountedDevice(size_t capacity) : capacity(capacity) {
		addListener(byteCountedDeviceListener = new ByteCountedDeviceListener(this));
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

	size_t getSizeFromNode(SRef<Node> node) {
		size_t count = 0;
		Node* n = node.get();
		if (!n) return 0;
		if (auto dir = dynamic_cast<MemDirectory*>(n)) {
			for (auto& child : dir->getChilds()) {
				count += child.length() + getSizeFromNode(dir->get(child));
			}
		} else if (auto file = dynamic_cast<MemFile*>(n)) {
			count += file->getSize();
		}
		return count;
	}

	size_t CodersFileSystem::MemDevice::getSize() const {
		return getSizeFromNode(root);
	}

	MemDevice::MemDevice(size_t capacity) : ByteCountedDevice(capacity) {
		root = new MemDirectory({listeners, ""}, checkSize);
	}

	SRef<FileStream> MemDevice::open(Path path, FileMode mode) {
		auto node = get(path);
		if (!node.isValid() && mode & FileMode::OUTPUT) {
			SRef<Directory> dir = get(path / "..");
			if (dir.isValid()) node = dir->createFile(path.fileName());
		}
		SRef<File> file = node;
		if (!file.isValid()) return nullptr;
		return file->open(mode);
	}

	SRef<Directory> MemDevice::createDir(Path path, bool createTree) {
		SRef<Directory> parent = root;
		while (!path.isSingle()) {
			SRef<Directory> newParent = get(Path(path.getRoot()));
			if (!newParent.isValid()) {
				if (createTree) {
					newParent = parent->createSubdir(path.getRoot());
				} else return nullptr;
			}
			parent = newParent;
			path = path.removeFrontNodes(1);
		}
		if (!parent.isValid()) return nullptr;
		return parent->createSubdir(path.getRoot());
	}

	bool MemDevice::remove(Path path, bool recursive) {
		SRef<Directory> d = get(path / "..");
		if (!d.isValid() || path.isRoot()) return false;
		return d->remove(path.fileName(), recursive);
	}

	bool MemDevice::rename(Path path, const std::string& name) {
		SRef<Directory> d = get(path / "..");
		if (!d.isValid() || path.isRoot() || !Path::isNode(name)) return false;
		return d->rename(path.fileName(), name);
	}

	SRef<Node> MemDevice::get(Path path) {
		path = path.absolute();
		if (path.isRoot()) return root;
		SRef<MemDirectory> dir = root;
		while (!path.isSingle() && dir.isValid()) {
			dir = dir->get(path.getRoot());
			path = path.removeFrontNodes(1);
		}
		if (dir.isValid()) return dir->get(path.getRoot());
		else return nullptr;
	}

	unordered_set<std::string> MemDevice::childs(Path path) {
		auto n = get(path);
		if (n.isValid()) return n->getChilds();
		return {};
	}

	size_t getSizeFromPath(fs::path e) {
		std::uint64_t count = 0;
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

	SRef<FileStream> DiskDevice::open(Path path, FileMode mode) {
		std::filesystem::path spath = realPath / path.relative().str();
		if (fs::exists(spath) && !fs::is_regular_file(spath)) return nullptr;
		else if (!fs::is_directory(spath / "..")) return nullptr;
		return new DiskFileStream(spath, mode, checkSize);
	}

	SRef<Directory> DiskDevice::createDir(Path path, bool createTree) {
		std::filesystem::path spath = realPath / path.relative().str();
		if (fs::exists(spath)) {
			if (fs::is_directory(spath)) return get(path);
			else return nullptr;
		}
		
		if (createTree) {
			fs::create_directories(spath);
		} else if (fs::is_directory(spath / "..")) {
			fs::create_directory(spath);
		} else return nullptr;
		tickWatcher();
		return get(path);
	}
#pragma optimize("",off)
	bool DiskDevice::remove(Path path, bool recursive) {
		if (path.isEmpty()) return false;
		std::filesystem::path spath = realPath / path.relative().str();
		try {
			if (recursive) return fs::remove_all(spath) > 0;
			else return fs::remove(spath);
			tickWatcher();
		} catch (...) {
			return false;
		}
	}
#pragma optimize("",on)

	bool DiskDevice::rename(Path path, const std::string& name) {
		if (path.isEmpty() || !Path::isNode(name)) return false;
		path = path.relative();
		std::filesystem::path spath = realPath / path.str();
		if (!fs::exists(spath) || fs::exists(realPath / (path / ".." / name).str()) || path.isRoot()) return false;
		fs::rename(spath, realPath / (path / ".." / name).str());
		tickWatcher();
		return true;
	}

	SRef<Node> DiskDevice::get(Path path) {
		if (path.isEmpty()) return new DiskDirectory(realPath, checkSize);
		std::filesystem::path spath = realPath / path.relative().str();
		if (fs::is_regular_file(spath)) {
			return new DiskFile(spath, checkSize);
		} else if (fs::is_directory(spath)) {
			return new DiskDirectory(spath, checkSize);
		}
		return nullptr;
	}

	unordered_set<std::string> DiskDevice::childs(Path path) {
		std::unordered_set<std::string> childs;
		std::filesystem::path spath = realPath / path.relative().str();
		std::filesystem::path NewPath = spath;
		for (const auto& entry : fs::directory_iterator(NewPath)) {
			std::string childName = entry.path().filename().string();
			childs.insert(childName);
		}
		return childs;
	}

	void DiskDevice::tickWatcher() {
		watcher.tick();
	}

	std::filesystem::path DiskDevice::getRealPath() const {
		return realPath;
	}

	DeviceNode::DeviceNode(SRef<Device> device) : device(device) {}

	SRef<FileStream> DeviceNode::open(FileMode mode) {
		return SRef<FileStream>();
	}

	unordered_set<std::string> DeviceNode::getChilds() const {
		return unordered_set<std::string>();
	}

	bool CodersFileSystem::DeviceNode::isValid() const {
		return true;
	}

	bool DeviceNode::mount(FileSystemRoot& fs, const Path& pathToDevice, const Path& pathToMountpoint) {
		SRef<DeviceNode> dev = fs.get(pathToDevice);
		if (!dev.isValid()) return false;
		return fs.mount(dev->device, pathToMountpoint);
	}
}