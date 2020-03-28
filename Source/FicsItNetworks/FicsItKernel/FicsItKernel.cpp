#include "FicsItKernel.h"

#include "Processor/Lua/LuaProcessor.h"

#include "SML/util/Logging.h"

namespace FicsItKernel {
	KernelCrash::KernelCrash(std::string what) : std::exception(what.c_str()) {}

	KernelCrash::~KernelCrash() {}

	KernelSystem::KernelSystem(UWorld* world) : world(world) {}

	KernelSystem::~KernelSystem() {}

	void KernelSystem::tick(float deltaSeconds) {
		if (getState() == RUNNING) processor->tick(deltaSeconds);
	}

	FileSystem::FileSystemRoot* KernelSystem::getFileSystem() {
		if (getState() == RUNNING) return &filesystem;
		return nullptr;
	}

	void KernelSystem::setCapacity(std::int64_t capacity) {
		memoryCapacity = capacity;
		if (getState() == RUNNING) start(true);
	}

	std::int64_t KernelSystem::getCapacity() const {
		return memoryCapacity;
	}

	void KernelSystem::setProcessor(Processor* processor) {
		this->processor = std::unique_ptr<Processor>(processor);
		if (getProcessor()) processor->setKernel(this);
	}

	Processor* KernelSystem::getProcessor() const {
		return processor.get();
	}

	KernelCrash KernelSystem::getCrash() const {
		return kernelCrash;
	}

	KernelState KernelSystem::getState() const {
		return state;
	}

	void KernelSystem::addDrive(AFINFileSystemState* drive) {
		// create & assign device from drive
		auto device = drive->createDevice();
		drives[drive] = TCHAR_TO_UTF8(*drive->ID.ToString());

		// check state and add device and drive to filesystem
		if (getState() != RUNNING) {
			devDevice->addDevice(driveToDevice[drive] = drive->createDevice(), TCHAR_TO_UTF8(*drive->ID.ToString()));
		}
	}

	void KernelSystem::removeDrive(AFINFileSystemState* drive) {
		// try to find location of drive
		auto s = drives.find(drive);
		if (s == drives.end()) return;

		// remove device from filesystem
		auto device = driveToDevice.find(s->first);
		devDevice->removeDevice(device->second);

		// remove drive from filesystem
		driveToDevice.erase(device);
		drives.erase(s);
	}

	bool KernelSystem::start(bool reset) {
		// check state and stop if needed
		if (getState() == RUNNING) {
			if (reset) {
				if (!stop()) return false;
			} else return false;
		} else if (getProcessor() == nullptr) return false;

		// set state and clear kernel crash
		state = RUNNING;
		kernelCrash = KernelCrash("");

		// reset whole system (filesystem, memory, processor, signal stuff)
		filesystem = FicsItFS::Root();
		devDevice = FileSystem::SRef<FicsItFS::DevDevice>(new FicsItFS::DevDevice());
		memoryUsage = 0;
		processor->reset();

		// add drives to filesystem
		auto drives = this->drives;
		this->drives.clear();
		for (auto& drive : drives) {
			addDrive(drive.first);
		}

		// create & mount root device
		if (this->drives.size() > 0) {
			// decide which and if a drive should get mounted
		} else {
			rootDevice = new FileSystem::MemDevice();
		}
		filesystem.mount(rootDevice, "/");

		// mount devDevice
		filesystem.mount(devDevice, "/dev");

		// finish start
		return true;
	}

	bool KernelSystem::stop() {
		// check state
		if (getState() != RUNNING) return false;

		// set state
		state = SHUTOFF;

		// clear filesystem
		filesystem = FicsItFS::Root();
		driveToDevice.clear();

		// finish stop
		return true;
	}

	void KernelSystem::crash(KernelCrash crash) {
		// check state
		if (getState() != RUNNING) return;

		// set state & crash
		state = CRASHED;
		kernelCrash = crash;
		SML::Logging::error("LUA Crash: ", kernelCrash.what());
	}

	void KernelSystem::recalculateResources(Recalc components) {
		memoryUsage = processor->getMemoryUsage(components & PROCESSOR);
		memoryUsage += filesystem.getMemoryUsage(components & FILESYSTEM);

		if (memoryUsage > memoryCapacity) crash({"out of memory"});
	}

	void KernelSystem::setNetwork(Network::NetworkController* controller) {
		network.reset(controller);
	}

	Network::NetworkController* KernelSystem::getNetwork() {
		return network.get();
	}

	std::int64_t KernelSystem::getMemoryUsage() {
		return memoryUsage;
	}

	void KernelSystem::signalFileSystemChange(int type, std::wstring npath, std::wstring opath) {
		network->pushSignalKernel("FileSystemChange", type, npath, opath);
	}
}