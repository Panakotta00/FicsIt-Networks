#include "Directory.h"

#include <fstream>

using namespace std;
using namespace FileSystem;
namespace filesystem = std::experimental::filesystem;

Directory::Directory() : Node() {}

Directory::~Directory() {}

MemDirectory::MemDirectory(ListenerListRef listeners, SizeCheckFunc checkSize) : Directory(), listeners(listeners), checkSize(checkSize) {}

MemDirectory::~MemDirectory() {}

SRef<Node> FileSystem::MemDirectory::get(const NodeName& name) {
	if (name.length() < 1) return nullptr;
	try {
		return entries.at(name);
	} catch (...) {
		return nullptr;
	}
}

unordered_set<NodeName> MemDirectory::getChilds() const {
	unordered_set<NodeName> nodes;
	for (auto& child : entries) nodes.insert(child.first);
	return nodes;
}

SRef<FileStream> MemDirectory::open(FileMode mode) {
	return nullptr;
}

bool FileSystem::MemDirectory::isValid() const {
	return true;
}

WRef<Directory> MemDirectory::createSubdir(const NodeName& subdir) {
	if (entries.find(subdir) != entries.end()) return entries[subdir];
	if (!checkSize(subdir.length(), true)) return nullptr;
	auto dir = new MemDirectory(ListenerListRef{listeners, subdir}, checkSize);
	entries[subdir] = dir;
	listeners.onNodeAdded(subdir, NT_Directory);
	return dir;
}

WRef<File> MemDirectory::createFile(const NodeName& name) {
	if (entries.find(name) != entries.end()) return entries[name];
	if (!checkSize(name.length(), true)) return nullptr;
	auto file = new MemFile(ListenerListRef{listeners, name}, checkSize);
	entries[name] = file;
	listeners.onNodeAdded(name, NT_File);
	return file;
}

bool MemDirectory::remove(const NodeName& entry, bool recursive) {
	bool ret = true;
	auto e_p = entries.find(entry);
	if (e_p == entries.end()) return false;
	SRef<Directory> dir = e_p->second;
	if (dir.isValid()) {
		if (!recursive) return false;
		for (auto& child : dir->getChilds()) {
			ret = ret & dir->remove(child, true);
		}
	}
	listeners.onNodeAdded(entry, getTypeFromRef(e_p->second));
	entries.erase(e_p);
	return true;
}

bool MemDirectory::rename(const NodeName& entry, const NodeName& name) {
	auto e_p = entries.find(entry);
	if (e_p == entries.end() || entries.find(name) != entries.end()) return false;
	if (entry.length() < name.length() && !checkSize(name.length() - entry.length(), true)) return false;
	entries[name] = e_p->second;
	listeners.onNodeRenamed(name, entry, getTypeFromRef(e_p->second));
	entries.erase(e_p);
	return true;
}

bool MemDirectory::add(const SRef<Node>& node, const NodeName& name) {
	if (!node.isValid()) return false;
	if (entries.find(name) != entries.end()) return false;
	if (!checkSize(name.length(), true)) return false;
	entries[name] = node;
	listeners.onNodeAdded(name, getTypeFromRef(node));
	return true;
}

DiskDirectory::DiskDirectory(const std::filesystem::path& realpath, SizeCheckFunc checkSize) : Directory(), realPath(realpath), checkSize(checkSize) {}

DiskDirectory::~DiskDirectory() {}

unordered_set<NodeName> DiskDirectory::getChilds() const {
	unordered_set<NodeName> nodes;
	for (auto e : filesystem::directory_iterator(realPath)) {
		nodes.insert(e.path().filename().generic_string()); 
	}
	return nodes;
}

SRef<FileStream> DiskDirectory::open(FileMode mode) {
	return nullptr;
}

bool DiskDirectory::isValid() const {
	return filesystem::is_directory(realPath);
}

WRef<Directory> DiskDirectory::createSubdir(const NodeName& subdir) {
	bool e = filesystem::exists(realPath / subdir);
	if (filesystem::is_directory(realPath / subdir) || !e) {
		if (!e) filesystem::create_directory(filesystem::absolute(realPath / subdir));
		return new DiskDirectory(realPath / subdir, checkSize);
	}
	return nullptr;
}

WRef<File> DiskDirectory::createFile(const NodeName& name) {
	if (!filesystem::is_regular_file(realPath / name) && filesystem::exists(realPath / name)) return nullptr;
	fstream f;
	f.open(realPath / name, fstream::out);
	f.close();
	return new DiskFile(realPath / name, checkSize);
}

bool DiskDirectory::remove(const NodeName& subdir, bool recursive) {
	bool isDir = filesystem::is_directory(realPath / subdir);
	if (filesystem::is_regular_file(realPath / subdir) || isDir) {
		try {
			if (recursive) filesystem::remove_all(realPath / subdir);
			else filesystem::remove(realPath / subdir);
		} catch (...) {
			return false;
		}
		return true;
	}
	return false;
}

bool FileSystem::DiskDirectory::rename(const NodeName& entry, const NodeName& name) {
	if (!filesystem::exists(realPath / entry) || filesystem::exists(name)) return false;
	filesystem::rename(realPath / entry, realPath / name);
	return true;
}
