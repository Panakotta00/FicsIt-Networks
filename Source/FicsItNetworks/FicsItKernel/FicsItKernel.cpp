#include "FicsItKernel.h"


#include "ArchiveUObjectFromStructuredArchive.h"
#include "Computer/FINComputerCase.h"
#include "FicsItNetworks/Graphics/FINGPUInterface.h"
#include "FicsItNetworks/Graphics/FINScreenInterface.h"
#include "Network/FINFuture.h"
#include "Processor/Lua/LuaProcessor.h"
#include "Reflection/FINReflection.h"

FFINKernelListener::FFINKernelListener(UFINKernelSystem* parent) : parent(parent) {}

void FFINKernelListener::onMounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) {
	static UFINSignal* Signal = nullptr;
	if (!Signal) Signal = FFINReflection::Get()->FindClass(AFINComputerCase::StaticClass())->FindFINSignal("FileSystemUpdate");
	Signal->Trigger(parent->GetOuter(), {4ll, FString(path.str().c_str())});
}

void FFINKernelListener::onUnmounted(FileSystem::Path path, FileSystem::SRef<FileSystem::Device> device) {
	static UFINSignal* Signal = nullptr;
	if (!Signal) Signal = FFINReflection::Get()->FindClass(AFINComputerCase::StaticClass())->FindFINSignal("FileSystemUpdate");
	Signal->Trigger(parent->GetOuter(), {5ll, FString(path.str().c_str())});
}

void FFINKernelListener::onNodeAdded(FileSystem::Path path, FileSystem::NodeType type) {
	static UFINSignal* Signal = nullptr;
	if (!Signal) Signal = FFINReflection::Get()->FindClass(AFINComputerCase::StaticClass())->FindFINSignal("FileSystemUpdate");
	Signal->Trigger(parent->GetOuter(), {0ll, FString(path.str().c_str()), static_cast<FINInt>(type)});
}

void FFINKernelListener::onNodeRemoved(FileSystem::Path path, FileSystem::NodeType type) {
	static UFINSignal* Signal = nullptr;
	if (!Signal) Signal = FFINReflection::Get()->FindClass(AFINComputerCase::StaticClass())->FindFINSignal("FileSystemUpdate");
	Signal->Trigger(parent->GetOuter(), {1ll, FString(path.str().c_str()), static_cast<FINInt>(type)});
}

void FFINKernelListener::onNodeChanged(FileSystem::Path path, FileSystem::NodeType type) {
	static UFINSignal* Signal = nullptr;
	if (!Signal) Signal = FFINReflection::Get()->FindClass(AFINComputerCase::StaticClass())->FindFINSignal("FileSystemUpdate");
	Signal->Trigger(parent->GetOuter(), {2ll, FString(path.str().c_str()), static_cast<FINInt>(type)});
}

void FFINKernelListener::onNodeRenamed(FileSystem::Path newPath, FileSystem::Path oldPath, FileSystem::NodeType type) {
	static UFINSignal* Signal = nullptr;
	if (!Signal) Signal = FFINReflection::Get()->FindClass(AFINComputerCase::StaticClass())->FindFINSignal("FileSystemUpdate");
	Signal->Trigger(parent->GetOuter(), {3ll, FString(newPath.str().c_str()), FString(oldPath.str().c_str()), static_cast<FINInt>(type)});
}

void UFINKernelSystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) {
	Super::AddReferencedObjects(InThis, Collector);
	
	UFINKernelSystem* This = Cast<UFINKernelSystem>(InThis);
	for (const TPair<void*, TFunction<void(void*, FReferenceCollector&)>>& Referencer : This->ReferencedObjects) {
		Referencer.Value(Referencer.Key, Collector);
	}
}

/*void UFINKernelSystem::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);

	if (!Ar.IsSaveGame()) return;

	FStructuredArchive::FRecord Record = FStructuredArchiveFromArchive(Ar).GetSlot().EnterRecord();

	// TODO: serialize kernel crash

	TOptional<FStructuredArchive::FSlot> Slot = Record.TryEnterField(FIELD_NAME_TEXT("FileSystem"), true);
	if (Slot.IsSet()) FileSystem.Serialize(Slot->GetUnderlyingArchive(), FileSystemSerializationInfo);

	if (GetProcessor()) GetProcessor()->SetKernel(this);
}*/

void UFINKernelSystem::Serialize(FStructuredArchive::FRecord Record) {
	Super::Serialize(Record);
	
	if (!Record.GetUnderlyingArchive().IsSaveGame()) return;

	// TODO: serialize kernel crash

	TOptional<FStructuredArchive::FSlot> FSSlot = Record.TryEnterField(FIELD_NAME_TEXT("FileSystem"), true);
	if (FSSlot.IsSet()) FileSystem.Serialize(FSSlot->GetUnderlyingArchive(), FileSystemSerializationInfo);

	//TOptional<FStructuredArchive::FSlot> ProcessorSlot = Record.TryEnterField(FIELD_NAME_TEXT("Processor"), true);
	//if (ProcessorSlot.IsSet()) ProcessorSlot->GetUnderlyingArchive() << Processor;

	if (GetProcessor()) GetProcessor()->SetKernel(this);
}

void UFINKernelSystem::BeginDestroy() {
	Stop();
	if (Processor) Processor->SetKernel(nullptr);
	Processor = nullptr;
	
	Super::BeginDestroy();
}

bool UFINKernelSystem::ShouldSave_Implementation() const {
	return true;
}

void UFINKernelSystem::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	SystemResetTimePoint = FTimespan::FromSeconds(GetWorld()->GetRealTimeSeconds()).GetTotalMilliseconds() - SystemResetTimePoint;
}

void UFINKernelSystem::PostSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	SystemResetTimePoint = FTimespan::FromSeconds(GetWorld()->GetRealTimeSeconds()).GetTotalMilliseconds() - SystemResetTimePoint;
}

void UFINKernelSystem::PreLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {}

void UFINKernelSystem::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	if (State == FIN_KERNEL_CRASHED || State == FIN_KERNEL_RUNNING) {
		if (State == FIN_KERNEL_CRASHED) Crash(MakeShared<FFINKernelCrash>());

		FileSystem = FFINKernelFSRoot();
		FileSystem.addListener(FileSystemListener);
		MemoryUsage = 0;

		// create & init devDevice
		DevDevice = new FFINKernelFSDevDevice();
		for (const TPair<AFINFileSystemState*, FileSystem::SRef<FileSystem::Device>>& Drive : Drives) {
			DevDevice->addDevice(Drive.Value, TCHAR_TO_UTF8(*Drive.Key->ID.ToString()));
		}
		
		if (DevDeviceMountPoint.Len() > 0) FileSystem.mount(DevDevice, TCHAR_TO_UTF8(*DevDeviceMountPoint));
		
		FileSystem.PostLoad(FileSystemSerializationInfo);
	}
	
	SystemResetTimePoint = FTimespan::FromSeconds(GetWorld()->GetRealTimeSeconds()).GetTotalMilliseconds() - SystemResetTimePoint;
}

void UFINKernelSystem::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	//if (Processor) out_dependentObjects.Add(Processor);
}

void UFINKernelSystem::Tick(float InDeltaSeconds) {
	if (GetState() == FIN_KERNEL_RESET) if (!Start(true)) return;
	if (GetState() == FIN_KERNEL_RUNNING) {
		if (DevDevice) DevDevice->tickListeners();
		if (Processor) Processor->Tick(InDeltaSeconds);
		else Crash(MakeShared<FFINKernelCrash>("Processor Unplugged"));
	}
}

FFINKernelFSRoot* UFINKernelSystem::GetFileSystem() {
	if (GetState() == FIN_KERNEL_RUNNING) return &FileSystem;
	return nullptr;
}

void UFINKernelSystem::SetCapacity(int64 Capacity) {
	MemoryCapacity = Capacity;
}

int64 UFINKernelSystem::GetCapacity() const {
	return MemoryCapacity;
}

void UFINKernelSystem::SetProcessor(UFINKernelProcessor* InProcessor) {
	Stop();
	if (GetProcessor()) {
		GetProcessor()->SetKernel(nullptr);
	}
	Processor = InProcessor;
	if (GetProcessor()) {
		InProcessor->SetKernel(this);
	}
}

UFINKernelProcessor* UFINKernelSystem::GetProcessor() const {
	return Processor;
}

TSharedPtr<FFINKernelCrash> UFINKernelSystem::GetCrash() const {
	return KernelCrash;
}

EFINKernelState UFINKernelSystem::GetState() const {
	return State;
}

void UFINKernelSystem::AddDrive(AFINFileSystemState* InDrive) {
	// check if drive is added & return if found
	if (Drives.Contains(InDrive)) return;

	// create & assign device from drive
	Drives.Add(InDrive) = InDrive->GetDevice();

	// add drive to devDevice
	if (DevDevice.isValid()) {
		if (!DevDevice->addDevice(Drives[InDrive], TCHAR_TO_UTF8(*InDrive->ID.ToString()))) Drives.Remove(InDrive);
	}
}

void UFINKernelSystem::RemoveDrive(AFINFileSystemState* InDrive) {
	// try to find location of drive
	FileSystem::SRef<FileSystem::Device>* FSDevice = Drives.Find(InDrive);
	if (!FSDevice) return;

	// remove drive from devDevice
	if (DevDevice.isValid()) {
		if (!DevDevice->removeDevice(*FSDevice)) return;
	}

	// unmount device
	FileSystem.unmount(*FSDevice);

	// remove drive from filesystem
	Drives.Remove(InDrive);
}

void UFINKernelSystem::PushFuture(TSharedPtr<TFINDynamicStruct<FFINFuture>> InFuture) {
	FutureQueue.Enqueue(InFuture);
}

void UFINKernelSystem::HandleFutures() {
	while (!FutureQueue.IsEmpty()) {
		TSharedPtr<TFINDynamicStruct<FFINFuture>> Future;
		FutureQueue.Peek(Future);
		FutureQueue.Pop();
		(*Future)->Execute();
	}
}

TMap<AFINFileSystemState*, FileSystem::SRef<FileSystem::Device>> UFINKernelSystem::GetDrives() const {
	return Drives;
}

FileSystem::SRef<FFINKernelFSDevDevice> UFINKernelSystem::GetDevDevice() const {
	return DevDevice;
}

bool UFINKernelSystem::InitFileSystem(FileSystem::Path InPath) {
	if (GetState() == FIN_KERNEL_RUNNING) {
		return FileSystem.mount(DevDevice, InPath);
	}
	return false;
}

bool UFINKernelSystem::Start(bool InReset) {
	// check state and stop if needed
	if (GetState() == FIN_KERNEL_RUNNING) {
		if (InReset) {
			if (!Stop()) return false;
		} else return false;
	}

	State = FIN_KERNEL_RUNNING;
	
	KernelCrash = MakeShared<FFINKernelCrash>("");

	// reset whole system (filesystem, memory, processor, signal stuff)
	FileSystem = FFINKernelFSRoot();
	FileSystem.addListener(FileSystemListener);
	MemoryUsage = 0;

	// create & init devDevice
	DevDevice = new FFINKernelFSDevDevice();
	for (const TPair<AFINFileSystemState*, FileSystem::SRef<FileSystem::Device>>& Drive : Drives) {
		DevDevice->addDevice(Drive.Value, TCHAR_TO_UTF8(*Drive.Key->ID.ToString()));
	}

	// check & reset processor
	if (GetProcessor() == nullptr) {
		Crash(MakeShared<FFINKernelCrash>("No processor set"));
		return false;
	}
	Processor->Reset();

	SystemResetTimePoint = FTimespan::FromSeconds(GetWorld()->GetRealTimeSeconds()).GetTotalMilliseconds();

	// finish start
	return true;
}

bool UFINKernelSystem::Reset() {
	if (Stop()) {
		State = FIN_KERNEL_RESET;
		return true;
	}
	return false;
}

bool UFINKernelSystem::Stop() {
	// set state
	State = FIN_KERNEL_SHUTOFF;

	// clear filesystem
	FileSystem = FFINKernelFSRoot();

	if (Processor) Processor->Stop(false);
	
	// finish stop
	return true;
}

void UFINKernelSystem::Crash(const TSharedRef<FFINKernelCrash>& InCrash) {
	// check state
	if (GetState() != FIN_KERNEL_RUNNING) return;

	// set state & crash
	State = FIN_KERNEL_CRASHED;
	KernelCrash = InCrash;

	if (Processor) Processor->Stop(true);
	
	if (GetDevDevice()) try {
		auto serial = GetDevDevice()->getSerial()->open(FileSystem::OUTPUT);
		if (serial) {
			*serial << "\r\n" << TCHAR_TO_UTF8(*KernelCrash->GetMessage()) << "\r\n";
			serial->close();
		}
	} catch (std::exception ex) {
		UE_LOG(LogFicsItNetworks, Error, TEXT("%s"), *FString(ex.what()));
	}
}

void UFINKernelSystem::RecalculateResources(ERecalc InComponents) {
	FileSystem::SRef<FFINKernelFSDevDevice> DevDevice = FileSystem.getDevDevice();
	
	MemoryUsage = Processor->GetMemoryUsage(InComponents & PROCESSOR);
	MemoryUsage += FileSystem.getMemoryUsage(InComponents & FILESYSTEM);
	if (DevDevice && DevDevice->getSerial().isValid()) MemoryUsage += DevDevice->getSerial()->getSize();
	if (MemoryUsage > MemoryCapacity) Crash(MakeShared<FFINKernelCrash>("out of memory"));
	if (DevDevice) DevDevice->updateCapacity(MemoryCapacity - MemoryUsage);
}

void UFINKernelSystem::SetNetwork(UFINKernelNetworkController* InController) {
	Network = InController;
}

UFINKernelNetworkController* UFINKernelSystem::GetNetwork() const {
	return Network;
}

UFINKernelAudioController* UFINKernelSystem::GetAudio() const {
	return Audio;
}

void UFINKernelSystem::SetAudio(UFINKernelAudioController* InController) {
	Audio = InController;
}

int64 UFINKernelSystem::GetMemoryUsage() const {
	return MemoryUsage;
}

void UFINKernelSystem::AddGPU(TScriptInterface<IFINGPUInterface> InGPU) {
	GPUs.Add(InGPU);
}

void UFINKernelSystem::RemoveGPU(TScriptInterface<IFINGPUInterface> InGPU) {
	GPUs.Remove(InGPU);
}

const TSet<TScriptInterface<IFINGPUInterface>>& UFINKernelSystem::GetGPUs() const {
	return GPUs;
}

void UFINKernelSystem::AddScreen(TScriptInterface<IFINScreenInterface> InScreen) {
	Screens.Add(InScreen);
}

void UFINKernelSystem::RemoveScreen(TScriptInterface<IFINScreenInterface> InScreen) {
	Screens.Remove(InScreen);
}

const TSet<TScriptInterface<IFINScreenInterface>>& UFINKernelSystem::GetScreens() const {
	return Screens;
}

int64 UFINKernelSystem::GetTimeSinceStart() const {
	return FTimespan(GetWorld()->GetRealTimeSeconds()).GetTotalMilliseconds() - SystemResetTimePoint;
}

void UFINKernelSystem::AddReferencer(void* Referencer, const TFunction<void(void*, FReferenceCollector&)>& CollectorFunc) {
	ReferencedObjects.FindOrAdd(Referencer) = CollectorFunc;
}

void UFINKernelSystem::RemoveReferencer(void* Referencer) {
	ReferencedObjects.Remove(Referencer);
}
