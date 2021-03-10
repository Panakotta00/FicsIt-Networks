#pragma once

#include "Library/FileSystemRoot.h"
#include "DevDevice.h"
#include "FileSystemSerializationInfo.h"

class FArchive;

/**
 * Extends the library file system root with:
 * - a central memory usage calculation
 * - preventing DevDevices to get unmounted
 * - preventing a second DevDevice to get mounted
 */
class FICSITNETWORKS_API FFINKernelFSRoot : public FileSystem::FileSystemRoot {
public:
	// Begin FileSystemRoot
	virtual bool mount(FileSystem::SRef<FileSystem::Device> device, FileSystem::Path path) override;
	virtual bool unmount(FileSystem::Path path) override;
	// End FileSystemRoot

	/**
	 * Unmounts the given device from the filesystem
	 *
	 * @param[in]	device	the device you want to unmounts
	 */
	bool unmount(FileSystem::SRef<FileSystem::Device> device);

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
	FileSystem::WRef<FFINKernelFSDevDevice> getDevDevice();

	/**
	 * Gets the mountpoint from a device
	 *
	 * @param[in]	device	the device you want to get the path from
	 * @return	the path of the device
	 */
	FileSystem::Path getMountPoint(FileSystem::SRef<FFINKernelFSDevDevice> device);

	/**
	 * Converts the given path into a string which is persistable.
	 *
	 * @param[in]	path	the path you want to convert
	 * @return	the path converted to persistable string.
	 */
	std::string persistPath(FileSystem::Path path);

	/**
	 * Converts a persisted path into a valid file system path.
	 *
	 * @param[in]	path	the persisted path as string
	 * @return	the unpersisted path
	 */
	FileSystem::Path unpersistPath(std::string path);

	/**
	 * Checks if the given path can get unpersisted
	 *
	 * @param[in]	path	the path you want to check
	 * @return	true if able to unpersist
	 */
	bool checkUnpersistPath(std::string path);

	/**
	 * Serializes the filesystem to an archive.
	 * Only sores mount points & tmpfs
	 * Make sure other used devices are already added before you deserialize.
	 *
	 * @param[in]	Ar		archive from which you want to read/write fro/to
	 * @param[in]	info	info storage were to store the readed data to
	 */
	void Serialize(FArchive& Ar, FFileSystemSerializationInfo& info);

	/**
	 * Reads the data from the info storage object
	 * and parses them to the filesystem as new mount mounts & devices
	 *
	 * @param[in]	info	the info storage holding the information about the FS State
	 */
	void PostLoad(const FFileSystemSerializationInfo& info);
};
