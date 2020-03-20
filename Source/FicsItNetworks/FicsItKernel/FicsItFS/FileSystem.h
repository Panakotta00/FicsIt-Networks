#pragma once

#include "Library/FileSystemRoot.h"

namespace FicsItKernel {
	namespace FicsItFS {
		class Root : public FileSystem::FileSystemRoot {
		protected:
			std::int64_t memoryUsage;

		public:
			bool remove(FileSystem::Path path, bool recursive = false);
			bool rename(FileSystem::Path path, const std::string& name);
			bool mount(FileSystem::SRef<FileSystem::Device> device, FileSystem::Path path);
			bool unmount(FileSystem::Path path);

			/**
			* Returns the memory consumption of the filesystem.
			* If recalc is set, forces the filesystem to recalculate the memory usage.
			*
			* @param[in]	recalc	forces the filesystem to recalculate the memory consumption
			* @return	returns the current memory consumption
			*/
			std::int64_t getMemoryUsage(bool recalc = false);
		};
	}
}
