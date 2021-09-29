#include "FileSystemRoot.h"

using namespace CodersFileSystem;
using namespace std;

FileSystemException::FileSystemException(std::string what) : std::exception(what.c_str()) {}

SRef<Device> FileSystemRoot::getDevice(Path path, Path& pending) {
	Path mountP = "";
	path = path.absolute();
	SRef<Device> mountD;
	for (auto mount = mounts.begin(); mount != mounts.end(); mount++) {
		if (!mount->second.first.isValid()) {
			mounts.erase(mount--);
			continue;
		}
		if (path.startsWith(mount->first) && mount->first.str().size() >= mountP.str().size()) {
			mountP = mount->first;
			mountD = mount->second.first;
		}
	}
	if (mountD.isValid()) {
		pending = path.str().erase(0, mountP.str().size());
	}
	pending = pending.relative();
	return mountD;
}

CodersFileSystem::FileSystemRoot::FileSystemRoot() {
	listener = new RootListener(this);
}

CodersFileSystem::FileSystemRoot::FileSystemRoot(FileSystemRoot&& other) {
	*this = std::move(other);
}

CodersFileSystem::FileSystemRoot::~FileSystemRoot() {}

FileSystemRoot& CodersFileSystem::FileSystemRoot::operator=(FileSystemRoot&& other) {
	mounts = other.mounts;
	cache = other.cache;
	listeners = other.listeners;
	listener = other.listener;
	listener->root = this;
	return *this;
}

SRef<FileStream> FileSystemRoot::open(Path path, FileMode mode) {
	Path pending = "";
	auto device = getDevice(path.absolute(), pending);
	if (!device.isValid()) return nullptr;
	return device->open(pending, mode);
}

SRef<Directory> FileSystemRoot::createDir(Path path, bool createTree) {
	Path pending = "";
	path = path.absolute();
	auto device = getDevice(path / "..", pending);
	if (!device.isValid()) return nullptr;
	return device->createDir(pending / path.fileName(), createTree);
}

bool FileSystemRoot::remove(Path path, bool recursive) {
	Path pending = "";
	path = path.absolute();
	auto device = getDevice(path / "..", pending);
	if (!device.isValid()) return false;
	auto removed = device->remove(pending / path.fileName(), recursive);
	if (removed) {
		for (auto i = mounts.begin(); i != mounts.end(); i++) {
			if (i->first.startsWith(path)) {
				mounts.erase(i--);
			}
		}
	}
	return true;
}

bool FileSystemRoot::rename(Path path, const std::string& name) {
	if (!Path::isNode(name)) return false;
	path = path.absolute();
	Path pending = "";
	auto device = getDevice(path / "..", pending);
	if (!device.isValid()) return false;
	auto renamed = device->rename(pending / path.fileName(), name);
	if (renamed) {
		for (auto i = mounts.begin(); i != mounts.end(); i++) {
			if (i->first.startsWith(path)) {
				auto newMountPathSub = i->first.relative().str().erase(0, path.relative().str().size());
				mounts[path / ".." / name / newMountPathSub] = i->second;
				mounts.erase(i--);
			}
		}
	}
	return true;
}

int FileSystemRoot::copy(Path from, Path to, bool recursive) {
	from = from.absolute();
	to = to.absolute();
	Path pendingFrom = "";
	Path pendingTo = "";
	auto deviceFrom = getDevice(from, pendingFrom);
	if (!deviceFrom.isValid()) return 1;
	auto deviceTo = getDevice(to, pendingTo);
	if (!deviceTo.isValid()) return 1;
	
	auto f = deviceFrom->get(pendingFrom);
	auto t = deviceTo->get(pendingTo);

	if (!recursive && dynamic_cast<Directory*>(f.get())) return 1;

	if (!t.isValid()) {
		SRef<Directory> prevT = deviceTo->get(pendingTo / "..");
		if (!prevT.isValid()) return 1;
		if (dynamic_cast<File*>(f.get())) {
			t = prevT->createFile(pendingTo.fileName());
		} else if (dynamic_cast<Directory*>(f.get())) {
			t = prevT->createSubdir(pendingTo.fileName());
		}
	} else if (from.fileName() != to.fileName()) {
		SRef<Directory> tDir = t;
		if (tDir.isValid()) {
			t = tDir->createSubdir(from.fileName());
			to = to / from.fileName();
		}
	}
	if (!t.isValid()) return 1;

	SRef<Directory> tDir = t;
	SRef<File> tFile = t;
	if (tDir.isValid()) {
		SRef<Directory> fDir = f;
		if (!fDir.isValid()) return 1;
		bool ret = true;
		for (auto& child : fDir->getChilds()) {
			if (copy(from / child, to / child)) ret = false;
		}
		return ret ? 0 : 2;
	} else if (tFile.isValid()) {
		auto ofs = tFile->open(OUTPUT);
		auto ifs = f->open(INPUT);
		if (!ofs.isValid() || !ifs.isValid()) return 1;
		ofs->write(FileStream::readAll(ifs));
		ofs->close();
	}
	return 1;
}

int FileSystemRoot::moveInternal(Path from, Path to) {
	Path pendingFrom = "";
	Path pendingTo = "";
	from = from.absolute();
	to = to.absolute();
	auto deviceFrom = getDevice(from, pendingFrom);
	if (!deviceFrom.isValid()) return 1;
	auto deviceTo = getDevice(to, pendingTo);
	if (!deviceTo.isValid()) return 1;

	auto f = deviceFrom->get(pendingFrom);
	auto t = deviceTo->get(pendingTo);

	if (!t.isValid()) {
		SRef<Directory> prevT = deviceTo->get(pendingTo / "..");
		if (!prevT.isValid()) return 1;
		if (dynamic_cast<File*>(f.get())) {
			t = prevT->createFile(pendingTo.fileName());
		} else if (dynamic_cast<Directory*>(f.get())) {
			t = prevT->createSubdir(pendingTo.fileName());
		}
	} else if (from.fileName() != to.fileName()) {
		SRef<Directory> tDir = t;
		if (tDir.isValid()) {
			if (dynamic_cast<File*>(f.get())) {
				t = tDir->createFile(from.fileName());
			} else if (dynamic_cast<Directory*>(f.get())) {
				t = tDir->createSubdir(from.fileName());
			}
			to = to / from.fileName();
		}
	}
	if (!t.isValid()) return 1;

	SRef<Directory> tDir = t;
	SRef<File> tFile = t;
	if (tDir.isValid()) {
		SRef<Directory> fDir = f;
		if (!fDir.isValid()) return 1;
		bool ret = true;
		for (auto& child : fDir->getChilds()) {
			bool able = moveInternal(from / child, to / child) > 0;
			if (!able) ret = false;
		}
		if (ret) remove(from, true);
		return ret ? 0 : 2;
	} else if (tFile.isValid()) {
		auto ofs = tFile->open(OUTPUT);
		auto ifs = f->open(INPUT);
		if (!ofs.isValid() || !ifs.isValid()) return 1;
		ofs->write(FileStream::readAll(ifs));
		ofs->close();
		ifs->close();
		return remove(from, false);
	}
	return 1;
}

int FileSystemRoot::move(Path from, Path to) {
	from = from.absolute();
	if (from.isRoot()) return 1;
	to = to.absolute();
	Path pendingFrom = "";
	auto deviceFrom = getDevice(from / "..", pendingFrom);
	if (!deviceFrom.isValid()) return 1;
	auto prevFrom = deviceFrom->get(pendingFrom);
	if (!prevFrom.isValid()) return 1;
	return moveInternal(from, to);
}

SRef<Node> FileSystemRoot::get(Path path) {
	path = path.absolute();
	auto cached_node = cache.find(path);
	if (cached_node != cache.end()) {
		if (!cached_node->second.isValid()) {
			cache.erase(cached_node);
		} else {
			return cached_node->second;
		}
	}
	Path pending = "";
	auto device = getDevice(path, pending);
	if (!device.isValid()) return nullptr;
	auto node = device->get(pending);
	if (!node.isValid()) return nullptr;
	return cache[path] = node;
}

unordered_set<std::string> FileSystemRoot::childs(Path path) {
	path = path.absolute();
	Path pending = "";
	auto device = getDevice(path, pending);
	if (!device.isValid()) throw FileSystemException("no device at path found");
	unordered_set<std::string> names = device->childs(pending);
	for (auto mount : mounts) {
		Path mountPoint = mount.first;
		if (!mountPoint.isRoot() && (mountPoint / "..") == path) names.insert(mountPoint.fileName());
	}
	return names;
}

bool FileSystemRoot::mount(SRef<Device> device, Path path) {
	path = path.absolute();
	for (auto& mount : mounts) {
		if (mount.first == path && mount.second.first == device) return false;
	}
	device->addListener((mounts[path] = {device, new PathBoundListener(listener, path)}).second);
	listener->onMounted(path, device);
	return true;
}

bool FileSystemRoot::unmount(Path path) {
	path = path.absolute();
	auto p = mounts.find(path);
	if (p == mounts.end()) return false;
	p->second.first->removeListener(p->second.second);
	listener->onUnmounted(path, p->second.first);
	mounts.erase(p);
	return true;
}

void FileSystemRoot::addListener(WRef<Listener> newListener) {
	listeners.insert(newListener);
}

void FileSystemRoot::removeListener(WRef<Listener> newListener) {
	listeners.erase(newListener);
}

FileSystemRoot::RootListener::RootListener(FileSystemRoot * root) : root(root) {}

CodersFileSystem::FileSystemRoot::RootListener::~RootListener() {}

void FileSystemRoot::RootListener::onMounted(Path path, SRef<Device> device) {
	for (auto i = root->cache.begin(); i != root->cache.end(); i++) if (i->first.startsWith(path)) root->cache.erase(i--);
	root->listeners.onMounted(path, device);
}

void FileSystemRoot::RootListener::onUnmounted(Path path, SRef<Device> device) {
	for (auto i = root->cache.begin(); i != root->cache.end(); i++) if (i->first.startsWith(path)) root->cache.erase(i--);
	root->listeners.onUnmounted(path, device);
}

void FileSystemRoot::RootListener::onNodeAdded(Path path, NodeType type) {
	root->listeners.onNodeAdded(path, type);
}

void FileSystemRoot::RootListener::onNodeRemoved(Path path, NodeType type) {
	root->cache.erase(path);
	root->listeners.onNodeRemoved(path, type);
}

void FileSystemRoot::RootListener::onNodeChanged(Path path, NodeType type) {
	root->listeners.onNodeChanged(path, type);
}

void CodersFileSystem::FileSystemRoot::RootListener::onNodeRenamed(Path newPath, Path oldPath, NodeType type) {
	try {
		root->cache[newPath] = root->cache.at(oldPath);
		root->cache.erase(oldPath);
	} catch (...) {}
	root->listeners.onNodeRenamed(newPath, oldPath, type);
}
