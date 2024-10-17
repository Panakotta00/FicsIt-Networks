#pragma once

#include "Library/Device.h"

class FICSITNETWORKSCOMPUTER_API FFINKernelFSDevDevice : public CodersFileSystem::Device {
private:
	std::unordered_map<std::string, CodersFileSystem::SRef<CodersFileSystem::Device>> Devices;

public:
	FFINKernelFSDevDevice();

	virtual CodersFileSystem::SRef<CodersFileSystem::FileStream> open(CodersFileSystem::Path path, CodersFileSystem::FileMode mode) override;
	virtual CodersFileSystem::SRef<CodersFileSystem::Node> get(CodersFileSystem::Path path) override;
	virtual bool remove(CodersFileSystem::Path path, bool recursive) override;
	virtual CodersFileSystem::SRef<CodersFileSystem::Directory> createDir(CodersFileSystem::Path, bool tree) override;
	virtual bool rename(CodersFileSystem::Path path, const std::string& name) override;
	virtual std::unordered_set<std::string> childs(CodersFileSystem::Path path) override;

	bool addDevice(CodersFileSystem::SRef<CodersFileSystem::Device> device, const std::string& name);
	bool removeDevice(CodersFileSystem::SRef<CodersFileSystem::Device> device);

	/**
	 * Gets a list of all devices and the corresponding names
	 *
	 * @return	a unordered map of devices with their names
	 */
	std::unordered_map<std::string, CodersFileSystem::SRef<CodersFileSystem::Device>> getDevices() const;

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
};
