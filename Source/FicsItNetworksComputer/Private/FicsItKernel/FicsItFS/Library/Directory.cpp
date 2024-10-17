#include "FicsItKernel/FicsItFS/Library/Directory.h"
#include <fstream>

using namespace std;
using namespace CodersFileSystem;

Directory::Directory() : Node() {}

Directory::~Directory() {}

MemDirectory::MemDirectory(ListenerListRef listeners, SizeCheckFunc checkSize) : Directory(), listeners(listeners), checkSize(checkSize) {}

MemDirectory::~MemDirectory() {}

SRef<Node> CodersFileSystem::MemDirectory::get(const std::string& name) {
	if (name.length() < 1) return nullptr;
	try {
		return entries.at(name);
	} catch (...) {
		return nullptr;
	}
}

unordered_set<std::string> MemDirectory::getChilds() const {
	unordered_set<std::string> nodes;
	for (auto& child : entries) nodes.insert(child.first);
	return nodes;
}

SRef<FileStream> MemDirectory::open(FileMode mode) {
	return nullptr;
}

bool CodersFileSystem::MemDirectory::isValid() const {
	return true;
}

WRef<Directory> MemDirectory::createSubdir(const std::string& subdir) {
	if (entries.find(subdir) != entries.end()) return entries[subdir];
	if (!checkSize(subdir.length(), true)) return nullptr;
	auto dir = new MemDirectory(ListenerListRef{listeners, Path(subdir)}, checkSize);
	entries[subdir] = dir;
	listeners.onNodeAdded(Path(subdir), NT_Directory);
	return dir;
}

WRef<File> MemDirectory::createFile(const std::string& name) {
	if (!Path::isNode(name)) return nullptr;
	if (entries.find(name) != entries.end()) return entries[name];
	if (!checkSize(name.length(), true)) return nullptr;
	auto file = new MemFile(ListenerListRef{listeners, Path(name)}, checkSize);
	entries[name] = file;
	listeners.onNodeAdded(Path(name), NT_File);
	return file;
}

bool MemDirectory::remove(const std::string& entry, bool recursive) {
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
	listeners.onNodeAdded(Path(entry), getTypeFromRef(e_p->second));
	entries.erase(e_p);
	return true;
}

bool MemDirectory::rename(const std::string& entry, const std::string& name) {
	if (!Path::isNode(name)) return false;
	auto e_p = entries.find(entry);
	if (e_p == entries.end() || entries.find(name) != entries.end()) return false;
	if (entry.length() < name.length() && !checkSize(name.length() - entry.length(), true)) return false;
	entries[name] = e_p->second;
	listeners.onNodeRenamed(Path(name), Path(entry), getTypeFromRef(e_p->second));
	entries.erase(e_p);
	return true;
}

bool MemDirectory::add(const SRef<Node>& node, const std::string& name) {
	if (!Path::isNode(name)) return false;
	if (!node.isValid()) return false;
	if (entries.find(name) != entries.end()) return false;
	if (!checkSize(name.length(), true)) return false;
	entries[name] = node;
	listeners.onNodeAdded(Path(name), getTypeFromRef(node));
	return true;
}

DiskDirectory::DiskDirectory(const std::filesystem::path& realpath, SizeCheckFunc checkSize) : Directory(), realPath(realpath), checkSize(checkSize) {}

DiskDirectory::~DiskDirectory() {}

unordered_set<std::string> DiskDirectory::getChilds() const {
	unordered_set<std::string> nodes;
	for (auto e : std::filesystem::directory_iterator(realPath)) {
		nodes.insert(e.path().filename().generic_string()); 
	}
	return nodes;
}

SRef<FileStream> DiskDirectory::open(FileMode mode) {
	return nullptr;
}

bool DiskDirectory::isValid() const {
	return std::filesystem::is_directory(realPath);
}

WRef<Directory> DiskDirectory::createSubdir(const std::string& subdir) {
	bool e = std::filesystem::exists(realPath / subdir);
	if (std::filesystem::is_directory(realPath / subdir) || !e) {
		if (!e) std::filesystem::create_directory(std::filesystem::absolute(realPath / subdir));
		return new DiskDirectory(realPath / subdir, checkSize);
	}
	return nullptr;
}

WRef<File> DiskDirectory::createFile(const std::string& name) {
	if (!Path::isNode(name)) return nullptr;
	if (!std::filesystem::is_regular_file(realPath / name) && std::filesystem::exists(realPath / name)) return nullptr;
	fstream f;
	f.open(realPath / name, fstream::out);
	f.close();
	return new DiskFile(realPath / name, checkSize);
}

bool DiskDirectory::remove(const std::string& subdir, bool recursive) {
	bool isDir = std::filesystem::is_directory(realPath / subdir);
	if (std::filesystem::is_regular_file(realPath / subdir) || isDir) {
		try {
			if (recursive) std::filesystem::remove_all(realPath / subdir);
			else std::filesystem::remove(realPath / subdir);
		} catch (...) {
			return false;
		}
		return true;
	}
	return false;
}

bool CodersFileSystem::DiskDirectory::rename(const std::string& entry, const std::string& name) {
	if (!Path::isNode(name)) return false;
	if (!std::filesystem::exists(realPath / entry) || std::filesystem::exists(realPath / name)) return false;
	std::filesystem::rename(realPath / entry, realPath / name);
	return true;
}
