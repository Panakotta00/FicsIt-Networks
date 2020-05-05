#pragma once

#include "Library/Device.h"
#include "Serial.h"

namespace FicsItKernel {
	namespace FicsItFS {
		class DevDevice : public FileSystem::Device {
		private:
			std::unordered_map<FileSystem::NodeName, FileSystem::SRef<FileSystem::Device>> devices;
			FileSystem::SRef<Serial> stdio;

		public:
			DevDevice();

			virtual FileSystem::SRef<FileSystem::FileStream> open(FileSystem::Path path, FileSystem::FileMode mode);
			virtual FileSystem::SRef<FileSystem::Node> get(FileSystem::Path path);
			virtual bool remove(FileSystem::Path path, bool recursive);
			virtual FileSystem::SRef<FileSystem::Directory> createDir(FileSystem::Path, bool tree);
			virtual bool rename(FileSystem::Path path, const FileSystem::NodeName& name);
			virtual std::unordered_set<FileSystem::NodeName> childs(FileSystem::Path path);

			bool addDevice(FileSystem::SRef<FileSystem::Device> device, const FileSystem::NodeName& name);
			bool removeDevice(FileSystem::SRef<FileSystem::Device> device);

			/**
			 * Gets a list of all devices and the corresponding names
			 *
			 * @return	a unordered map of devices with their names
			 */
			std::unordered_map<FileSystem::NodeName, FileSystem::SRef<FileSystem::Device>> getDevices();

			/**
			 * Iterates over every MemDevice added to the Device-List.
			 * Updates their capacity to their current memory usage + given capacity.
			 *
			 * @param	capacity	new capacity buffer for all MemDevices
			 */
			void updateCapacity(std::int64_t capacity);

			/**
			 * Ticks all disk listeners
			 */
			void tickListeners();

			/**
			 * Returns the memory file used as standard input & output
			 */
			FileSystem::SRef<Serial> getSerial();
		};
	}
}