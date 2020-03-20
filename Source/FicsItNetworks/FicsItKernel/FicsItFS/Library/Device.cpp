#include "Device.h"

#include <iostream>

#include "FileSystemRoot.h"

using namespace std;
namespace fs = std::filesystem;
namespace FileSystem {
	ByteCountedDevice::ByteCountedDeviceListener::ByteCountedDeviceListener(ByteCountedDevice* root) : root(root) {}

	void FileSystem::ByteCountedDevice::ByteCountedDeviceListener::onMounted(Path path, SRef<Device> device) {
		if (root->listenerMask & 0b000001) root->usedValid = false;
	}

	void FileSystem::ByteCountedDevice::ByteCountedDeviceListener::onUnmounted(Path path, SRef<Device> device) {
		if (root->listenerMask & 0b000010) root->usedValid = false;
	}

	void FileSystem::ByteCountedDevice::ByteCountedDeviceListener::onNodeAdded(Path path, NodeType type) {
		if (root->listenerMask & 0b000100) root->usedValid = false;
	}

	void FileSystem::ByteCountedDevice::ByteCountedDeviceListener::onNodeRemoved(Path path, NodeType type) {
		if (root->listenerMask & 0b001000) root->usedValid = false;
	}

	void FileSystem::ByteCountedDevice::ByteCountedDeviceListener::onNodeChanged(Path path, NodeType type) {
		if (root->listenerMask & 0b010000) root->usedValid = false;
	}

	void FileSystem::ByteCountedDevice::ByteCountedDeviceListener::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
		if (root->listenerMask & 0b100000) root->usedValid = false;
	}

	void Device::addListener(WRef<Listener> listener) {
		listeners.insert(listener);
	}

	void Device::removeListener(WRef<Listener> listener) {
		listeners.erase(listener);
	}

	bool ByteCountedDevice::checkSizeFunc(size_t size, bool addIfAble) {
		if (capacity < 1) return true;
		if (used + size > capacity) return false;
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

	size_t FileSystem::MemDevice::getSize() const {
		return getSizeFromNode(root);
	}

	MemDevice::MemDevice(size_t capacity) : ByteCountedDevice(capacity) {
		root = new MemDirectory({listeners, ""}, checkSize);
	}

	SRef<FileStream> MemDevice::open(Path path, FileMode mode) {
		auto node = get(path);
		if (!node.isValid()) {
			SRef<Directory> dir = get(path.prev());
			if (dir.isValid()) node = dir->createFile(path.getFinal());
		}
		SRef<File> file = node;
		if (!file.isValid()) return nullptr;
		return file->open(mode);
	}

	SRef<Directory> MemDevice::createDir(Path path, bool createTree) {
		SRef<Directory> parent = root;
		while (!path.isFinal()) {
			SRef<Directory> newParent = get(path.getRoot());
			if (!newParent.isValid()) {
				if (createTree) {
					newParent = parent->createSubdir(path.getRoot());
				} else return nullptr;
			}
			parent = newParent;
			path = path.next();
		}
		if (!parent.isValid()) return nullptr;
		return parent->createSubdir(path.getRoot());
	}

	bool MemDevice::remove(Path path, bool recursive) {
		SRef<Directory> d = get(path.prev());
		if (!d.isValid()) return false;
		return d->remove(path.getFinal(), recursive);
	}

	bool MemDevice::rename(Path path, const std::string& name) {
		SRef<Directory> d = get(path.prev());
		if (!d.isValid()) return false;
		return d->rename(path.getFinal(), name);
	}

	SRef<Node> MemDevice::get(Path path) {
		if (path.getNodeCount() < 1) return root;
		SRef<MemDirectory> dir = root;
		while (!path.isFinal() && dir.isValid()) {
			dir = dir->get(path.getRoot());
			path = path.next();
		}
		if (dir.isValid()) return dir->get(path.getRoot());
		else return nullptr;
	}

	unordered_set<string> MemDevice::childs(Path path) {
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
		if (fs::exists(realPath / path) && !fs::is_regular_file(realPath / path)) return nullptr;
		else if (!fs::is_directory(realPath / path.prev())) return nullptr;
		return new DiskFileStream(realPath / path, mode, checkSize);
	}

	SRef<Directory> DiskDevice::createDir(Path path, bool createTree) {
		if (fs::exists(realPath / path)) {
			if (fs::is_directory(realPath / path)) return get(path);
			else return nullptr;
		} else if (!createTree && !fs::is_directory(realPath / path.prev())) return nullptr;
		if (createTree) {
			fs::create_directories(realPath / path);
		} else if (fs::is_directory(realPath / path.prev())) {
			fs::create_directory(realPath / path);
		} else return nullptr;
		tickWatcher();
		return get(path);
	}

	bool DiskDevice::remove(Path path, bool recursive) {
		try {
			if (recursive) return fs::remove_all(realPath / path) > 0;
			else return fs::remove(realPath / path);
			tickWatcher();
		} catch (...) {
			return false;
		}
	}

	bool DiskDevice::rename(Path path, const std::string& name) {
		if (!fs::exists(realPath / path) || fs::exists(realPath / path.prev() / name) || path.getNodeCount() < 1) return false;
		fs::rename(realPath / path, realPath / path.prev() / name);
		tickWatcher();
		return true;
	}

	SRef<Node> DiskDevice::get(Path path) {
		if (path.getNodeCount() < 1) return new DiskDirectory(realPath, checkSize);
		if (fs::is_regular_file(realPath / path)) {
			return new DiskFile(realPath / path, checkSize);
		} else if (fs::is_directory(realPath / path)) {
			return new DiskDirectory(realPath / path, checkSize);
		}
		return nullptr;
	}

	unordered_set<string> DiskDevice::childs(Path path) {
		std::unordered_set<std::string> childs;
		for (const auto& entry : fs::directory_iterator(realPath / path))
			childs.insert(entry.path().filename().string());
		return childs;
	}

	void DiskDevice::tickWatcher() {
		watcher.tick();
	}

	DeviceNode::DeviceNode(SRef<Device> device) : device(device) {}

	SRef<FileStream> DeviceNode::open(FileMode mode) {
		return SRef<FileStream>();
	}

	unordered_set<string> DeviceNode::getChilds() const {
		return unordered_set<string>();
	}

	bool FileSystem::DeviceNode::isValid() const {
		return true;
	}

	bool DeviceNode::mount(FileSystemRoot& fs, const Path& pathToDevice, const Path& pathToMountpoint) {
		SRef<DeviceNode> dev = fs.get(pathToDevice);
		if (!dev.isValid()) return false;
		return fs.mount(dev->device, pathToMountpoint);
	}
}