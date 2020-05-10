#pragma once

#include <memory>
#include <queue>

#include "Processor/Processor.h"
#include "FicsItFS/FINFileSystemState.h"
#include "FicsItFS/DevDevice.h"
#include "FicsItFS/FileSystem.h"
#include "Network/NetworkController.h"
#include "Json.h"

namespace FicsItKernel {
	enum KernelState {
		SHUTOFF,
		RUNNING,
		CRASHED,
		RESET
	};

	class KernelCrash : public std::exception {
	public:
		KernelCrash(std::string what = "");

		virtual ~KernelCrash();
	};

	class KernelSystem;
	class KernelListener : public FileSystem::Listener {
	private:
		KernelSystem* parent;
	
	public:
		KernelListener(KernelSystem* parent);

		virtual void onMounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) override;
		virtual void onUnmounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) override;
		virtual void onNodeAdded(FileSystem::Path path, FileSystem::NodeType type) override;
		virtual void onNodeRemoved(FileSystem::Path path, FileSystem::NodeType type) override;
		virtual void onNodeChanged(FileSystem::Path  path, FileSystem::NodeType type) override;
		virtual void onNodeRenamed(FileSystem::Path newPath, FileSystem::Path oldPath, FileSystem::NodeType type) override;
	};

	class KernelSystem {
		friend Processor;

	private:
		KernelState state = KernelState::SHUTOFF;
		KernelCrash kernelCrash;
		std::int64_t memoryCapacity = 0;
		std::int64_t memoryUsage = 0;
		std::unique_ptr<Processor> processor = nullptr;
		FicsItFS::Root filesystem;
		FileSystem::SRef<FicsItFS::DevDevice> devDevice = nullptr;
		std::unordered_map<AFINFileSystemState*, FileSystem::SRef<FileSystem::Device>> drives;
		std::unique_ptr<Network::NetworkController> network = nullptr;
		UWorld* world = nullptr;
		FileSystem::SRef<KernelListener> listener;
		TSharedPtr<FJsonObject> readyToUnpersist = nullptr;

	public:
		/**
		 * defines which resource usage should get recalculated
		 */
		enum Recalc {
			NONE = 0b00,
			PROCESSOR = 0b01,
			FILESYSTEM = 0b10,
			ALL = 0b11
		};

		KernelSystem(UWorld* world);
		~KernelSystem();

		/**
		 * Ticks the whole system.
		 *
		 * @param	deltaSeconds	time in seconds since last tick
		 */
		void tick(float deltaSeconds);

		/**
		 * Allows to access the filesystem of the kernel.
		 * Returns nullptr if it is not running or if it is not setup.
		 *
		 * @return	returns the current running filesystem as pointer
		 */
		FicsItFS::Root* getFileSystem();

		/**
		 * Sets the memory capacity to the given value.
		 * If the system is running, causes a hard reset.
		 *
		 * @param	capacity	the new memory capacity
		 */
		void setCapacity(std::int64_t capacity);

		/**
		 * Returns the current memory capacity.
		 *
		 * @return	current memory capacits
		 */
		std::int64_t getCapacity() const;

		/**
		 * Sets the processor.
		 * If the system is running, causes a hard reset.
		 * The given pointer will get fully occupied and so, manual deletion is not allowed.
		 *
		 * @param	processor	the process you want now use
		 */
		void setProcessor(Processor* processor);

		/**
		 * Returns the currently use processor
		 */
		Processor* getProcessor() const;

		/**
		 * Returns the current kernel crash.
		 *
		 * @return	current kernel crash
		 */
		KernelCrash getCrash() const;

		/**
		 * Returns the current kernel state.
		 *
		 * @return	current kernel state
		 */
		KernelState getState() const;

		/**
		 * Adds the given drive to the system.
		 *
		 * @param	drive	the drive you want to add
		 */
		void addDrive(AFINFileSystemState* drive);

		/**
		 * Removes the given drive from the system.
		 *
		 * @param	drive	the drive you want to remove
		 */
		void removeDrive(AFINFileSystemState* drive);

		/**
		 * Gets the internally used DevDevice
		 *
		 * @return	the used DevDevice
		 */
		FileSystem::SRef<FicsItFS::DevDevice> getDevDevice();

		/**
		 * Mounts the currently used devDevice to the given path in the currently used file system.
		 *
		 * @param	path	path were the DevDevice should get mounted to
		 * @return	true if it was able to mount the DevDevice, fals if not (f.e. when the DevDevice got already mounted in this run state)
		 */
		bool initFileSystem(FileSystem::Path path);

		/**
		 * Starts the system.
		 * If the system is already running, resets the system if given reset is set.
		 * Crashes the system if no processor is set.
		 *
		 * @param	reset	set it to true if you want to allow a system reset
		 * @return	returns flase if system is already running and reset is not set, else it returns true
		 */
		bool start(bool reset);

		/**
		 * Resets the system.
		 * This is like start(true) but it is capable of getting called within a system tick.
		 * It basically stops the system and changes the state to reset. In the next tick the system will then get started again.
		 *
		 * @return	returns false if system was not able to get stopped
		 */
		bool reset();

		/**
		 * Stops the system.
		 *
		 * @return	returns false if system is already not running, else it returns true
		 */
		bool stop();

		/**
		 * Crashes the system with the given kernel crash.
		 *
		 * @param	crash	the kernel crash wich is the reason for the crash
		 */
		void crash(KernelCrash crash);

		/**
		 * Returns the currently used network controller.
		 */
		Network::NetworkController* getNetwork();

		/**
		 * Sets the currently used network controller.
		 * Completely occupys the controller, that means, it gets manged and so you should never free the controller.
		 */
		void setNetwork(Network::NetworkController* controller);

		/**
		 * Returns the currently used unreal world.
		 */
		UWorld* getWorld();

		/**
		 * Get current used memory.
		 *
		 * @return	current memory usage
		 */
		int64 getMemoryUsage();

		/**
		 * Recalculates the given system components resource usage like memory.
		 * Can cause a kernel crash to occur.
		 * Trys to use cached memory usages for the components not set.
		 *
		 * @param	components	the registry of system components you want to recalculate.
		 */
		void recalculateResources(Recalc components);

		/**
		 * Persists the current system state to a json object.
		 *
		 * @return	the kernel state as a json object
		 */
		TSharedPtr<FJsonObject> persist();

		/**
		 * Unpersists the system state given as a json object.
		 * Might cause the system to directly start.
		 * If not processor is set, sets the processor state json object
		 * to get automatically unpersited when a processor get set.
		 *
		 * @param	state	the system state as json object
		 */
		void unpersist(TSharedPtr<FJsonObject> state);
	};
}
