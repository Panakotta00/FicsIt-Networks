#pragma once

#include "FicsItFileSystem/Device.h"

class FICSITFILESYSTEM_API FFINKernelFSDevDevice : public CodersFileSystem::Device {
private:
	std::unordered_map<std::string, TSharedRef<Device>> Devices;

public:
	FFINKernelFSDevDevice();

	virtual TSharedPtr<CodersFileSystem::FileStream> open(CodersFileSystem::Path path, CodersFileSystem::FileMode mode) override;
	virtual bool remove(CodersFileSystem::Path path, bool recursive) override;
	virtual bool createDir(CodersFileSystem::Path, bool tree) override;
	virtual bool rename(CodersFileSystem::Path path, const std::string& name) override;
	virtual std::unordered_set<std::string> children(CodersFileSystem::Path path) override;
	virtual TSharedPtr<Device> getDevice(CodersFileSystem::Path path) override;
	virtual TOptional<CodersFileSystem::FileType> fileType(CodersFileSystem::Path path) override;
	virtual void tickListeners();

	bool addDevice(TSharedRef<Device> device, const std::string& name);
	bool removeDevice(TSharedRef<Device> device);

	/**
	 * Gets a list of all devices and the corresponding names
	 *
	 * @return	a unordered map of devices with their names
	 */
	std::unordered_map<std::string, TSharedRef<Device>> getDevices() const;

	/**
	 * Iterates over every MemDevice added to the Device-List.
	 * Updates their capacity to their current memory usage + given capacity.
	 *
	 * @param	capacity	new capacity buffer for all MemDevices
	 */
	void updateCapacity(std::int64_t capacity);
};
