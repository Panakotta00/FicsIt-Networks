#pragma once

#include "Library/Device.h"

namespace FicsItKernel {
	namespace FicsItFS {
		class DevDevice : public FileSystem::Device {
		private:
			std::unordered_map<std::string, FileSystem::SRef<FileSystem::Device>> devices;

		public:
			DevDevice();

			virtual FileSystem::SRef<FileSystem::FileStream> open(FileSystem::Path path, FileSystem::FileMode mode);
			virtual FileSystem::SRef<FileSystem::Node> get(FileSystem::Path path);
			virtual bool remove(FileSystem::Path path, bool recursive);
			virtual FileSystem::SRef<FileSystem::Directory> createDir(FileSystem::Path, bool tree);
			virtual bool rename(FileSystem::Path path, const std::string& name);
			virtual std::unordered_set<std::string> childs(FileSystem::Path path);

			bool addDevice(FileSystem::SRef<FileSystem::Device> device, const std::string& name);
			bool removeDevice(FileSystem::SRef<FileSystem::Device> device);
		};
	}
}