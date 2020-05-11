#include "FicsItKernel.h"

#include "Processor/Lua/LuaProcessor.h"

#include "SML/util/Logging.h"

#include "Json.h"

namespace FicsItKernel {
	KernelCrash::KernelCrash(std::string what) : std::exception(what.c_str()) {}

	KernelCrash::~KernelCrash() {}

	KernelSystem::KernelSystem() : listener(new KernelListener(this)) {}

	KernelSystem::~KernelSystem() {}

	void KernelSystem::tick(float deltaSeconds) {
		if (getState() == RESET) if (!start(true)) return;
		if (getState() == RUNNING) {
			if (devDevice) devDevice->tickListeners();
			if (processor) processor->tick(deltaSeconds);
			else crash(FicsItKernel::KernelCrash("Processor Unplugged"));
		}
	}

	FicsItFS::Root* KernelSystem::getFileSystem() {
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
		if (getProcessor()) {
			processor->setKernel(this);
			if (readyToUnpersist.IsValid()) unpersist(readyToUnpersist);
		}
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
		// check if drive is added & return if found
		if (drives.find(drive) != drives.end()) return;

		// create & assign device from drive
		drives[drive] = drive->createDevice();

		// add drive to devDevice
		if (devDevice.isValid()) {
			if (!devDevice->addDevice(drives[drive], TCHAR_TO_UTF8(*drive->ID.ToString()))) drives.erase(drive);
		}
	}

	void KernelSystem::removeDrive(AFINFileSystemState* drive) {
		// try to find location of drive
		auto s = drives.find(drive);
		if (s == drives.end()) return;

		// remove drive from devDevice
		if (devDevice.isValid()) {
			if (!devDevice->removeDevice(s->second)) return;
		}

		// remove drive from filesystem
		drives.erase(s);
	}

	FileSystem::SRef<FicsItFS::DevDevice> KernelSystem::getDevDevice() {
		return devDevice;
	}

	bool KernelSystem::initFileSystem(FileSystem::Path path) {
		if (getState() == RUNNING) {
			return filesystem.mount(devDevice, path);
		} else return false;
	}

	bool KernelSystem::start(bool reset) {
		// check state and stop if needed
		if (getState() == RUNNING) {
			if (reset) {
				if (!stop()) return false;
			} else return false;
		}

		state = RUNNING;
		
		if (getProcessor() == nullptr) {
			crash(KernelCrash("No processor set"));
			return false;
		}
		
		kernelCrash = KernelCrash("");

		// reset whole system (filesystem, memory, processor, signal stuff)
		filesystem = FicsItFS::Root();
		filesystem.addListener(listener);
		memoryUsage = 0;
		processor->reset();

		// create & init devDevice
		devDevice = new FicsItFS::DevDevice();
		for (auto& drive : drives) {
			devDevice->addDevice(drive.second, TCHAR_TO_UTF8(*drive.first->ID.ToString()));
		}

		// finish start
		return true;
	}

	bool KernelSystem::reset() {
		if (stop()) {
			state = RESET;
			return true;
		}
		return false;
	}

	bool KernelSystem::stop() {
		// check state
		if (getState() != RUNNING) return false;

		// set state
		state = SHUTOFF;

		// clear filesystem
		filesystem = FicsItFS::Root();

		// finish stop
		return true;
	}

	void KernelSystem::crash(KernelCrash crash) {
		// check state
		if (getState() != RUNNING) return;

		// set state & crash
		state = CRASHED;
		kernelCrash = crash;
		
		try {
			auto serial = getDevDevice()->getSerial()->open(FileSystem::OUTPUT);
			if (serial) {
				*serial << "\r\n" << kernelCrash.what() << "\r\n";
				serial->close();
			}
		} catch (std::exception ex) {
			SML::Logging::error(ex.what());
		}
	}

	void KernelSystem::recalculateResources(Recalc components) {
		memoryUsage = processor->getMemoryUsage(components & PROCESSOR);
		memoryUsage += filesystem.getMemoryUsage(components & FILESYSTEM);
		memoryUsage += devDevice->getSerial()->getSize();

		if (memoryUsage > memoryCapacity) crash({"out of memory"});
		FileSystem::SRef<FicsItFS::DevDevice> dev = filesystem.getDevDevice();
		if (dev) dev->updateCapacity(memoryCapacity - memoryUsage);
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

	TSharedPtr<FJsonObject> KernelSystem::persist() {
		TSharedPtr<FJsonObject> json = MakeShareable(new FJsonObject());
		switch(getState()) {
		case SHUTOFF:
			json->SetNumberField("SystemState", 0);
			break;
		case RUNNING:
			json->SetNumberField("SystemState", 1);
			break;
		case CRASHED:
			json->SetNumberField("SystemState", 2);
			break;
		case RESET:
			json->SetNumberField("SystemState", 3);
			break;
		}

		if (processor) {
			TSharedPtr<FJsonObject> processorState = processor->persist();
			json->SetObjectField("ProcessorState", processorState);
		}
		
		return json;
    }

	void KernelSystem::unpersist(TSharedPtr<FJsonObject> json) {
		if (processor) {
			switch(json->GetIntegerField("SystemState")) {
			case 0:
                state = SHUTOFF;
				break;
			case 1:
                start(true);
				break;
			case 2:
                state = CRASHED;
				break;
			case 3:
                state = RESET;
				break;
			}

			try {
				processor->unpersist(json->GetObjectField("ProcessorState"));
			} catch (...) {
				stop();
				SML::Logging::error("Unable to unpersist computer state! Network: '", TCHAR_TO_UTF8(*getNetwork()->getComponent()->GetPathName()), "'");
			}
		} else {
			readyToUnpersist = json;
		}
    }

	KernelListener::KernelListener(KernelSystem* parent) : parent(parent) {}

	void KernelListener::onMounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 4, path.str());
	}

	void KernelListener::onUnmounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 5, path.str());
	}

	void KernelListener::onNodeAdded(FileSystem::Path path, FileSystem::NodeType type) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 0, path.str(), (int)type);
	}

	void KernelListener::onNodeRemoved(FileSystem::Path path, FileSystem::NodeType type) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 1, path.str(), (int)type);
	}

	void KernelListener::onNodeChanged(FileSystem::Path path, FileSystem::NodeType type) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 2, path.str(), (int)type);
	}

	void KernelListener::onNodeRenamed(FileSystem::Path newPath, FileSystem::Path oldPath, FileSystem::NodeType type) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 3, newPath.str(), oldPath.str(), (int)type);
	}
}