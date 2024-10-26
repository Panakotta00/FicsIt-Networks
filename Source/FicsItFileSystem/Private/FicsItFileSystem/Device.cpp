#include "FicsItFileSystem/Device.h"
#include "FicsItFileSystem/FileSystemRoot.h"
#include <iostream>
#include <cstdint>

#include "FicsItFileSystem.h"
#include "FicsItLogLibrary.h"
#include "StructuredLog.h"

using namespace std;
namespace fs = std::filesystem;

// TODO: Add verbose FileSystem logging

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
		std::error_code code;
		if (fs::exists(spath, code) && !fs::is_regular_file(spath, code)) {
			if (!fs::is_directory(spath)) {
				UFILogLibrary::Log(FIL_Verbosity_Warning, FString::Printf(TEXT("Tried to open mapped host path '%s' as regular file, but it is neither a regular file nor a directory."), UTF8_TO_TCHAR(spath.c_str())));
				return nullptr;
			}
			UE_LOGFMT(LogFicsItFileSystem, Verbose, "Tried to open mapped host path '{path}' as regular file, but it is not a regular file: {error}", FString(UTF8_TO_TCHAR(spath.c_str())), FString(UTF8_TO_TCHAR(code.message().c_str())));
			return nullptr;
		}
		if (!fs::is_directory(spath.parent_path(), code)) {
			UE_LOGFMT(LogFicsItFileSystem, Verbose, "Tried to open mapped host path '{path}' as regular file, but its parent folder is not a directory: {error}", FString(UTF8_TO_TCHAR(spath.c_str())), FString(UTF8_TO_TCHAR(code.message().c_str())));
			return nullptr;
		}
		return MakeShared<DiskFileStream>(spath, mode, checkSize);
	}

	bool DiskDevice::createDir(Path path, bool createTree) {
		std::filesystem::path spath = realPath / path.relative().str();
		std::error_code code;
		if (fs::exists(spath, code)) {
			if (fs::is_directory(spath, code)) return true;
			else return false;
		}

		if (createTree) {
			fs::create_directories(spath, code);
		} else if (fs::is_directory(spath.parent_path(), code)) {
			fs::create_directory(spath, code);
		} else return false;
		tickListeners();
		return true;
	}

	bool DiskDevice::remove(Path path, bool recursive) {
		std::error_code code;
		if (path.isEmpty()) return false;
		std::filesystem::path spath = realPath / path.relative().str();
		try {
			if (recursive) return fs::remove_all(spath, code) > 0;
			else return fs::remove(spath, code);
			tickListeners();
		} catch (...) {
			return false;
		}
	}

	bool DiskDevice::rename(Path path, const std::string& name) {
		std::error_code code;
		if (path.isEmpty() || !Path::isNode(name)) return false;
		path = path.relative();
		std::filesystem::path spath = realPath / path.str();
		if (!fs::exists(spath, code) || fs::exists(realPath / (path / ".." / name).str(), code) || path.isRoot()) return false;
		fs::rename(spath, realPath / (path / ".." / name).str(), code);
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
		std::error_code code;
		auto fspath = realPath / path.relative().str();
		if (std::filesystem::is_directory(fspath, code)) {
			return File_Directory;
		} else if (std::filesystem::is_regular_file(fspath, code)) {
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