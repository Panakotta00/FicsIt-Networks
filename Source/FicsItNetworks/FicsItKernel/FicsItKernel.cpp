#include "FicsItKernel.h"

#include "KernelSystemSerializationInfo.h"
#include "FicsItNetworks/Graphics/FINGPUInterface.h"
#include "FicsItNetworks/Graphics/FINScreenInterface.h"
#include "Network/FINFuture.h"
#include "Processor/Lua/LuaProcessor.h"
#include "SML/util/Logging.h"

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
		drives[drive] = drive->GetDevice();

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

		// unmount device
		filesystem.unmount(s->second);

		// remove drive from filesystem
		drives.erase(s);
	}

	void KernelSystem::pushFuture(TSharedPtr<TFINDynamicStruct<FFINFuture>> future) {
		futureQueue.push(future);
	}

	void KernelSystem::handleFutures() {
		while (futureQueue.size() > 0) {
			TSharedPtr<TFINDynamicStruct<FFINFuture>> future = futureQueue.front();
			futureQueue.pop();
			(*future)->Execute();
		}
	}

	std::unordered_map<AFINFileSystemState*, FileSystem::SRef<FileSystem::Device>> KernelSystem::getDrives() const {
		return drives;
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
		
		kernelCrash = KernelCrash("");

		// reset whole system (filesystem, memory, processor, signal stuff)
		filesystem = FicsItFS::Root();
		filesystem.addListener(listener);
		memoryUsage = 0;

		// create & init devDevice
		devDevice = new FicsItFS::DevDevice();
		for (auto& drive : drives) {
			devDevice->addDevice(drive.second, TCHAR_TO_UTF8(*drive.first->ID.ToString()));
		}

		// check & reset processor
		if (getProcessor() == nullptr) {
			crash(KernelCrash("No processor set"));
			return false;
		}
		processor->reset();
		

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
		
		if (getDevDevice()) try {
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

	Audio::AudioController* KernelSystem::getAudio() {
		return audio.get();
	}

	void KernelSystem::setAudio(Audio::AudioController* controller) {
		audio.reset(controller);
	}

	Network::NetworkController* KernelSystem::getNetwork() {
		return network.get();
	}

	std::int64_t KernelSystem::getMemoryUsage() {
		return memoryUsage;
	}

	void KernelSystem::addGPU(UObject* gpu) {
		check(gpu->GetClass()->ImplementsInterface(UFINGPUInterface::StaticClass()))
		gpus.Add(gpu);
	}

	void KernelSystem::removeGPU(UObject* gpu) {
		gpus.Remove(gpu);
	}

	TSet<UObject*> KernelSystem::getGPUs() {
		TSet<UObject*> set;
		for (const FWeakObjectPtr& ptr : gpus) set.Add(ptr.Get());
		return set;
	}

	void KernelSystem::addScreen(UObject* screen) {
		check(screen->GetClass()->ImplementsInterface(UFINScreenInterface::StaticClass()))
        auto i = screens.Find(screen);
		if (!i) screens.Add(screen);
	}

	void KernelSystem::removeScreen(UObject* screen) {
		screens.Remove(screen);
	}
	
	TSet<UObject*> KernelSystem::getScreens() {
		TSet<UObject*> set;
		for (const FWeakObjectPtr& ptr : screens) set.Add(ptr.Get());
		return set;
	}

	void KernelSystem::PreSerialize(FKernelSystemSerializationInfo& Data, bool bLoading) {
		Data.bPreSerialized = true;

		// pre serialize network
		network->PreSerialize(bLoading);
		
		// pre serialize processor
		if (processor.get() != nullptr) {
			Data.processorState = processor->CreateSerializationStorage();
			processor->PreSerialize(Data.processorState, bLoading);
		}
	}
	
	void KernelSystem::Serialize(FArchive& Ar, FKernelSystemSerializationInfo& OutSystemState) {
		if (!Ar.IsSaveGame() || !OutSystemState.bPreSerialized) return;
		
		// serialize system state
		OutSystemState.systemState = state;
		Ar << OutSystemState.systemState;

		// serialize kernel crash
		OutSystemState.crash = kernelCrash.what();
		Ar << OutSystemState.crash;

		// serialize processor
		bool proc = (processor.get() != nullptr) && OutSystemState.processorState;
		Ar << proc;
		if (Ar.IsSaving()) {
			// Serialize processor
			if (proc) {
				TSubclassOf<UProcessorStateStorage> storageClass = OutSystemState.processorState->GetClass();
				Ar << storageClass;
				processor->Serialize(OutSystemState.processorState, false);
				OutSystemState.processorState->Serialize(Ar);
			}
		} else if (Ar.IsLoading()) {
			// Deserialize processor
			if (proc) {
				checkf(processor.get(), L"Tryed to unpersist processor state but no processor set");
				UClass* storageClass = nullptr;
				Ar << storageClass;
				OutSystemState.processorState = Cast<UProcessorStateStorage>(NewObject<UProcessorStateStorage>(GetTransientPackage(), storageClass));
				OutSystemState.processorState->Serialize(Ar);
				if (processor.get()) processor->Serialize(OutSystemState.processorState, true);
			}
		}
		
		// Serialize Network
		network->Serialize(Ar);

		OutSystemState.devDeviceMountPoint = UTF8_TO_TCHAR(filesystem.getMountPoint(devDevice).str().c_str());
		Ar << OutSystemState.devDeviceMountPoint;
		
		// Serialize FileSystem
		filesystem.Serialize(Ar, OutSystemState.fileSystemState);
	}

	void KernelSystem::PostSerialize(FKernelSystemSerializationInfo& Data, bool bLoading) {
		state = static_cast<KernelState>(Data.systemState);
		if (bLoading && (state == CRASHED || state == RUNNING)) {
			start(true);
			if (state == CRASHED) crash(KernelCrash(TCHAR_TO_UTF8(*Data.crash)));
			
			if (Data.devDeviceMountPoint.Len() > 0) filesystem.mount(devDevice, TCHAR_TO_UTF8(*Data.devDeviceMountPoint));
			
			filesystem.PostLoad(Data.fileSystemState);
		}
		
		if (Data.processorState || processor.get()) {
			processor->PostSerialize(Data.processorState, bLoading);
		}
		Data.processorState = nullptr;

		network->PostSerialize(bLoading);

		Data.bPreSerialized = false;
	}

	KernelListener::KernelListener(KernelSystem* parent) : parent(parent) {}

	void KernelListener::onMounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 4ll, path.str().c_str());
	}

	void KernelListener::onUnmounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 5ll, path.str().c_str());
	}

	void KernelListener::onNodeAdded(FileSystem::Path path, FileSystem::NodeType type) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 0ll, path.str().c_str(), (FINInt)type);
	}

	void KernelListener::onNodeRemoved(FileSystem::Path path, FileSystem::NodeType type) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 1ll, path.str().c_str(), (FINInt)type);
	}

	void KernelListener::onNodeChanged(FileSystem::Path path, FileSystem::NodeType type) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 2ll, path.str().c_str(), (FINInt)type);
	}

	void KernelListener::onNodeRenamed(FileSystem::Path newPath, FileSystem::Path oldPath, FileSystem::NodeType type) {
		parent->getNetwork()->pushSignalKernel("FileSystemUpdate", 3ll, newPath.str().c_str(), oldPath.str().c_str(), (FINInt)type);
	}
}
