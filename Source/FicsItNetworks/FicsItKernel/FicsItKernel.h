#pragma once

#include <memory>
#include <queue>

#include "Processor/Processor.h"
#include "FicsItFS/FINFileSystemState.h"
#include "FicsItFS/DevDevice.h"
#include "FicsItFS/FileSystem.h"
#include "Network/NetworkController.h"

namespace FicsItKernel {
	enum KernelState {
		SHUTOFF,
		RUNNING,
		CRASHED,
	};

	class KernelCrash : public std::exception {
	public:
		KernelCrash(std::string what = "");

		virtual ~KernelCrash();
	};

	class KernelSystem {
		friend Processor;

	private:
		KernelState state = KernelState::SHUTOFF;
		KernelCrash kernelCrash;
		std::int32_t memoryCapacity = 0;
		std::int32_t memoryUsage = 0;
		std::unique_ptr<Processor> processor;
		FicsItFS::Root filesystem;
		FileSystem::SRef<FicsItFS::DevDevice> devDevice;
		FileSystem::SRef<FileSystem::Device> rootDevice;
		std::unordered_map<AFINFileSystemState*, std::string> drives;
		std::unordered_map<AFINFileSystemState*, FileSystem::SRef<FileSystem::Device>> driveToDevice;
		std::unique_ptr<Network::NetworkController> network;
		UWorld* world;

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
		FileSystem::FileSystemRoot* getFileSystem();

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
		 * Starts the system.
		 * If the system is already running, resets the system if given reset is set.
		 *
		 * @param	reset	set it to true if you want to allow a system reset
		 * @return	returns flase if system is already running and reset is not set, else it returns true
		 */
		bool start(bool reset);

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
		 * Emits a file system change signal
		 *
		 * @param	type	defines the actual change event in the FS: 0 fileCreated, 1 fileDeleted, 2 fileContentChanged, 3 fileRenamed, 4 dirCreated, 5 dirDeleted, 6 dirRenamed
		 * @param	npath	the new path of the node
		 * @param	opath	the old path of the node
		 */
		void signalFileSystemChange(int type, std::wstring npath, std::wstring opath);
	};
}
