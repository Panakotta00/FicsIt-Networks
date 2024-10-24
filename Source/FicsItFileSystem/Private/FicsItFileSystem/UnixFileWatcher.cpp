#include "FicsItFileSystem/UnixFileWatcher.h"

#if PLATFORM_UNIX

using namespace CodersFileSystem;

FileWatcher::FileWatcher(const std::filesystem::path& path, std::function<void(int, NodeType, Path, Path)> event) : eventFunc(event), realPath(path) {}
FileWatcher::~FileWatcher() {}
void FileWatcher::tick() {}

#endif
