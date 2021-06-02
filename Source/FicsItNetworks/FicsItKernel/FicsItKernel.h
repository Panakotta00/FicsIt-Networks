#pragma once

#include "Audio/AudioController.h"
#include "Dom/JsonObject.h"
#include "FicsItFS/FINFileSystemState.h"
#include "FicsItFS/DevDevice.h"
#include "FicsItNetworks/Computer/FINPciDeviceInterface.h"
#include "Network/NetworkController.h"
#include "FicsItNetworks/Graphics/FINGPUInterface.h"
#include "FicsItNetworks/Graphics/FINScreenInterface.h"
#include "FicsItNetworks/Network/FINFuture.h"
#include "FicsItNetworks/Utils/FINException.h"
#include "FicsItKernel.generated.h"

class UFINKernelProcessor;
UENUM()
enum EFINKernelState {
	FIN_KERNEL_SHUTOFF,
	FIN_KERNEL_RUNNING,
	FIN_KERNEL_CRASHED,
	FIN_KERNEL_RESET
};

USTRUCT()
struct FICSITNETWORKS_API FFINKernelCrash : public FFINException {
	GENERATED_BODY()
	
	FFINKernelCrash() = default;
	FFINKernelCrash(const FString& Message) : FFINException(Message) {}
};

class UFINKernelSystem;

class FICSITNETWORKS_API FFINKernelListener : public CodersFileSystem::Listener {
private:
	UFINKernelSystem* parent;

public:
	FFINKernelListener(UFINKernelSystem* parent);

	virtual void onMounted(CodersFileSystem::Path path, CodersFileSystem::SRef<CodersFileSystem::Device> device) override;
	virtual void onUnmounted(CodersFileSystem::Path path, CodersFileSystem::SRef<CodersFileSystem::Device> device) override;
	virtual void onNodeAdded(CodersFileSystem::Path path, CodersFileSystem::NodeType type) override;
	virtual void onNodeRemoved(CodersFileSystem::Path path, CodersFileSystem::NodeType type) override;
	virtual void onNodeChanged(CodersFileSystem::Path  path, CodersFileSystem::NodeType type) override;
	virtual void onNodeRenamed(CodersFileSystem::Path newPath, CodersFileSystem::Path oldPath, CodersFileSystem::NodeType type) override;
};

UCLASS()
class FICSITNETWORKS_API UFINKernelSystem : public UObject, public IFGSaveInterface {
	GENERATED_BODY()
	
private:
	// System Setup
	UPROPERTY(SaveGame)
	UFINKernelProcessor* Processor = nullptr;
	UPROPERTY()
	UFINKernelNetworkController* Network = nullptr;
	UPROPERTY()
	UFINKernelAudioController* Audio = nullptr;
	TArray<TScriptInterface<IFINPciDeviceInterface>> PCIDevices;
	FFINKernelFSRoot FileSystem;
	CodersFileSystem::SRef<FFINKernelFSDevDevice> DevDevice = nullptr;
	int64 MemoryCapacity = 0;
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<AFINFileSystemState*, CodersFileSystem::SRef<CodersFileSystem::Device>> Drives;

	// Runtime Environment/State
	UPROPERTY(SaveGame)
	TEnumAsByte<EFINKernelState> State = FIN_KERNEL_SHUTOFF;
	UPROPERTY(SaveGame)
	uint64 SystemResetTimePoint = 0;
	UPROPERTY(SaveGame)
	FString DevDeviceMountPoint;
	TSharedPtr<FFINKernelCrash> KernelCrash;
	int64 MemoryUsage = 0;
	CodersFileSystem::SRef<FFINKernelListener> FileSystemListener;

	// Cache
	TSharedPtr<FJsonObject> ReadyToUnpersist = nullptr;
	TQueue<TSharedPtr<TFINDynamicStruct<FFINFuture>>> FutureQueue;
	TMap<void*, TFunction<void(void*, FReferenceCollector&)>> ReferencedObjects;
	FFileSystemSerializationInfo FileSystemSerializationInfo;
	
public:
	/**
	 * defines which resource usage should get recalculated
	 */
	enum ERecalc {
		NONE = 0b00,
		PROCESSOR = 0b01,
		FILESYSTEM = 0b10,
		ALL = 0b11
	};

	UFINKernelSystem();

	// Begin UObject
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	//virtual void Serialize(FArchive& Ar) override;
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	virtual void BeginDestroy() override;
	// End UObject

	// Begin IFGSaveInterface
	virtual bool ShouldSave_Implementation() const override;
	virtual void PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) override;
	virtual void GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) override;
	// End IFGSaveInterface

	/**
	 * Ticks the whole system.
	 *
	 * @param	DeltaSeconds	time in seconds since last tick
	 */
	void Tick(float DeltaSeconds);

	/**
	 * Allows to access the filesystem of the kernel.
	 * Returns nullptr if it is not running or if it is not setup.
	 *
	 * @return	returns the current running filesystem as pointer
	 */
	FFINKernelFSRoot* GetFileSystem();

	/**
	 * Sets the memory capacity to the given value.
	 * If the system is running, causes a hard reset.
	 *
	 * @param	InCapacity	the new memory capacity
	 */
	void SetCapacity(int64 InCapacity);

	/**
	 * Returns the current memory capacity.
	 *
	 * @return	current memory capacity
	 */
	int64 GetCapacity() const;

	/**
	 * Sets the processor.
	 * If the system is running, causes a hard reset.
	 * The given pointer will get fully occupied and so, manual deletion is not allowed.
	 *
	 * @param	InProcessor	the process you want now use
	 */
	void SetProcessor(UFINKernelProcessor* InProcessor);

	/**
	 * Returns the currently use processor
	 */
	UFINKernelProcessor* GetProcessor() const;

	/**
	 * Returns the current kernel crash.
	 *
	 * @return	current kernel crash
	 */
	TSharedPtr<FFINKernelCrash> GetCrash() const;

	/**
	 * Returns the current kernel state.
	 *
	 * @return	current kernel state
	 */
	EFINKernelState GetState() const;

	/**
	 * Adds the given drive to the system.
	 *
	 * @param	Drive	the drive you want to add
	 */
	void AddDrive(AFINFileSystemState* Drive);

	/**
	 * Removes the given drive from the system.
	 *
	 * @param	Drive	the drive you want to remove
	 */
	void RemoveDrive(AFINFileSystemState* Drive);

	/**
	 * Adds a future to resolve to the future queue.
	 * So it gets resolved in on of the next main thread ticks.
	 *
	 * @param[in]	Future	shared ptr to the future you want to resolve
	 */
	void PushFuture(TSharedPtr<TFINDynamicStruct<FFINFuture>> Future);

	/**
	 * This function should get executed every main thread tick.
	 * @note	ONLY FROM THE MAIN THREAD!!!
	 */
	void HandleFutures();

	/**
	 * Returns all drive added to the kernel
	 *
	 * @return	the drives
	 */
	TMap<AFINFileSystemState*, CodersFileSystem::SRef<CodersFileSystem::Device>> GetDrives() const;

	/**
	 * Gets the internally used DevDevice
	 *
	 * @return	the used DevDevice
	 */
	CodersFileSystem::SRef<FFINKernelFSDevDevice> GetDevDevice() const;

	/**
	 * Mounts the currently used devDevice to the given path in the currently used file system.
	 *
	 * @param	InPath	path were the DevDevice should get mounted to
	 * @return	true if it was able to mount the DevDevice, false if not (f.e. when the DevDevice got already mounted in this run state)
	 */
	bool InitFileSystem(CodersFileSystem::Path InPath);

	/**
	 * Starts the system.
	 * If the system is already running, resets the system if given reset is set.
	 * Crashes the system if no processor is set.
	 *
	 * @param	bInReset	set it to true if you want to allow a system reset
	 * @return	returns false if system is already running and reset is not set, else it returns true
	 */
	bool Start(bool bInReset);

	/**
	 * Resets the system.
	 * This is like start(true) but it is capable of getting called within a system tick.
	 * It basically stops the system and changes the state to reset. In the next tick the system will then get started again.
	 *
	 * @return	returns false if system was not able to get stopped
	 */
	bool Reset();

	/**
	 * Stops the system.
	 *
	 * @return	returns false if system is already not running, else it returns true
	 */
	bool Stop();

	/**
	 * Crashes the system with the given kernel crash.
	 *
	 * @param	InCrash	the kernel crash which is the reason for the crash
	 */
	void Crash(const TSharedRef<FFINKernelCrash>& InCrash);

	/**
	 * Returns the currently used network controller.
	 */
	UFINKernelNetworkController* GetNetwork() const;

	/**
	 * Sets the currently used network controller.
	 * Completely occupies the controller, that means, it gets manged and so you should never free the controller.
	 */
	void SetNetwork(UFINKernelNetworkController* InController);

	/**
	 * Returns the currently used audio controller.
	 */
	UFINKernelAudioController* GetAudio() const;

	/**
	 * Sets the currently used audio controller.
	 * Completely occupies the controller, that means, it gets managed and so you should never free the controller.
	 */
	void SetAudio(UFINKernelAudioController* InController);

	/**
	 * Get current used memory.
	 *
	 * @return	current memory usage
	 */
	int64 GetMemoryUsage() const;

	/**
	 * Adds the given PCI Device to the kernel
	 *
	 * @param[in]	InPCIDevice		the PCI Device you want to add.
	 */
	void AddPCIDevice(TScriptInterface<IFINPciDeviceInterface> InPCIDevice);

	/**
	* Removes the given GPU from the kernel
	*
	* @param[in]	InPCIDevice		the PCI Device you want to remove.
	*/
	void RemovePCIDevice(TScriptInterface<IFINPciDeviceInterface> InPCIDevice);

	/**
	 * Returns the list of added PCI Devices.
	 *
	 * @return list of added PCI Devices
	 */
	const TArray<TScriptInterface<IFINPciDeviceInterface>>& GetPCIDevices() const;

	/**
	 * Returns the amount of milliseconds passed since the system started.
	 *
	 * @return	amount of milliseconds since system start
	 */
	int64 GetTimeSinceStart() const;
	
	/**
	 * Recalculates the given system components resource usage like memory.
	 * Can cause a kernel crash to occur.
	 * Tries to use cached memory usages for the components not set.
	 *
	 * @param	InComponents	the registry of system components you want to recalculate.
	 */
	void RecalculateResources(ERecalc InComponents);

	/**
	 * Adds a new referencer to the referencer storage
	 */
	void AddReferencer(void* Referencer, const TFunction<void(void*, FReferenceCollector&)>& CollectorFunc);

	/**
	 * Removes teh given referencer from the referencer storage
	 */
	void RemoveReferencer(void* Referencer);
};
