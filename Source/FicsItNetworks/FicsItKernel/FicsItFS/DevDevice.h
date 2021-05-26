#pragma once

#include "Library/Device.h"
#include "Serial.h"

class FICSITNETWORKS_API FFINKernelFSDevDevice : public CodersFileSystem::Device {
private:
	std::unordered_map<CodersFileSystem::NodeName, CodersFileSystem::SRef<CodersFileSystem::Device>> Devices;
	CodersFileSystem::SRef<FFINKernelFSSerial> Serial;

public:
	FFINKernelFSDevDevice();

	virtual CodersFileSystem::SRef<CodersFileSystem::FileStream> open(CodersFileSystem::Path path, CodersFileSystem::FileMode mode) override;
	virtual CodersFileSystem::SRef<CodersFileSystem::Node> get(CodersFileSystem::Path path) override;
	virtual bool remove(CodersFileSystem::Path path, bool recursive) override;
	virtual CodersFileSystem::SRef<CodersFileSystem::Directory> createDir(CodersFileSystem::Path, bool tree) override;
	virtual bool rename(CodersFileSystem::Path path, const CodersFileSystem::NodeName& name) override;
	virtual std::unordered_set<CodersFileSystem::NodeName> childs(CodersFileSystem::Path path) override;

	bool addDevice(CodersFileSystem::SRef<CodersFileSystem::Device> device, const CodersFileSystem::NodeName& name);
	bool removeDevice(CodersFileSystem::SRef<CodersFileSystem::Device> device);

	/**
	 * Gets a list of all devices and the corresponding names
	 *
	 * @return	a unordered map of devices with their names
	 */
	std::unordered_map<CodersFileSystem::NodeName, CodersFileSystem::SRef<CodersFileSystem::Device>> getDevices() const;

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
	CodersFileSystem::SRef<FFINKernelFSSerial> getSerial() const;
};
