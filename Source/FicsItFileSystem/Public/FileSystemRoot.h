#pragma once

#include "FicsItFileSystem/FileSystemRoot.h"
#include "DevDevice.h"
#include "FileSystemSerializationInfo.h"

class FArchive;

/**
 * Extends the library file system root with:
 * - a central memory usage calculation
 * - preventing DevDevices to get unmounted
 * - preventing a second DevDevice to get mounted
 */
class FICSITFILESYSTEM_API FFINKernelFSRoot : public CodersFileSystem::FileSystemRoot {
public:
	// Begin FileSystemRoot
	virtual bool mount(TSharedRef<CodersFileSystem::Device> device, CodersFileSystem::Path path) override;
	virtual bool unmount(CodersFileSystem::Path path) override;
	// End FileSystemRoot

	/**
	 * Unmounts the given device from the filesystem
	 *
	 * @param[in]	device	the device you want to unmounts
	 */
	bool unmount(TSharedRef<CodersFileSystem::Device> device);

	/**
	* Returns the memory consumption of the filesystem.
	* If recalc is set, forces the filesystem to recalculate the memory usage.
	*
	* @param[in]	recalc	forces the filesystem to recalculate the memory consumption
	* @return	returns the current memory consumption
	*/
	int64 getMemoryUsage(bool recalc = false);

	/**
	 * Gets the mountpoint from a device
	 *
	 * @param[in]	device	the device you want to get the path from
	 * @return	the path of the device
	 */
	CodersFileSystem::Path getMountPoint(TSharedRef<FFINKernelFSDevDevice> device);

	/**
	 * Serializes the filesystem to an archive.
	 * Only sores mount points & tmpfs
	 * Make sure other used devices are already added before you deserialize.
	 *
	 * @param[in]	Ar		archive from which you want to read/write fro/to
	 * @param[in]	info	info storage were to store the readed data to
	 */
	void Serialize(FStructuredArchive::FRecord Record, FFileSystemSerializationInfo& info);

	/**
	 * Reads the data from the info storage object
	 * and parses them to the filesystem as new mount mounts & devices
	 *
	 * @param[in]	info	the info storage holding the information about the FS State
	 */
	void PostLoad(const FFileSystemSerializationInfo& info);
};
