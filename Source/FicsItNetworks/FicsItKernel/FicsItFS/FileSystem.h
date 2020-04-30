#pragma once

#include "Library/FileSystemRoot.h"
#include "DevDevice.h"

namespace FicsItKernel {
	namespace FicsItFS {
		/**
		 * Extends the library file system root with:
		 * - a centeral memory usage calculation
		 * - preventing DevDevices to get unmounted
		 * - preventing a seccond DevDevice to get mounted
		 */
		class Root : public FileSystem::FileSystemRoot {
		public:

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

			/**
			 * Searchs in all mounts for a DevDevice mount
			 *
			 * @return	the found DevDevice, nullptr if not found
			 */
			FileSystem::WRef<DevDevice> getDevDevice();
		};
	}
}
