#include "Computer/FINComputerCase.h"
#include "Computer/FINComputerEEPROMDesc.h"
#include "Computer/FINComputerFloppyDesc.h"
#include "FicsItKernel/FicsItKernel.h"
#include "Computer/FINComputerDriveHolder.h"
#include "Computer/FINComputerMemory.h"
#include "Computer/FINComputerProcessor.h"
#include "FGInventoryComponent.h"
#include "FGPlayerController.h"
#include "Computer/FINComputerRCO.h"
#include "Engine/ActorChannel.h"
#include "FicsItKernel/Logging.h"
#include "Net/UnrealNetwork.h"
#include "Utils/FINUtils.h"

class UFINComputerRCO;

AFINComputerCase::AFINComputerCase() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->OnNetworkSignal.AddDynamic(this, &AFINComputerCase::HandleSignal);
	NetworkConnector->SetIsReplicated(true);
	
	Panel = CreateDefaultSubobject<UFINModuleSystemPanel>("Panel");
	Panel->SetupAttachment(RootComponent);
	Panel->OnModuleChanged.AddDynamic(this, &AFINComputerCase::OnModuleChanged);

	DataStorage = CreateDefaultSubobject<UFGInventoryComponent>("DataStorage");
	DataStorage->SetDefaultSize(2);
	DataStorage->Resize(2);
	DataStorage->OnItemRemovedDelegate.AddDynamic(this, &AFINComputerCase::OnEEPROMChanged);
	DataStorage->OnItemAddedDelegate.AddDynamic(this, &AFINComputerCase::OnEEPROMChanged);
	DataStorage->mItemFilter.BindLambda([](TSubclassOf<UObject> Item, int32 Index) {
		switch (Index) {
		case -1:
			return Item->IsChildOf(UFINComputerEEPROMDesc::StaticClass()) || Item->IsChildOf(UFINComputerFloppyDesc::StaticClass());
		case 0:
			return Item->IsChildOf(UFINComputerEEPROMDesc::StaticClass());
		case 1:
			return Item->IsChildOf(UFINComputerFloppyDesc::StaticClass());
		default:
			return false;
		}
	});
	DataStorage->SetIsReplicated(true);

	Speaker = CreateDefaultSubobject<UAudioComponent>("Speaker");
	Speaker->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	Log = CreateDefaultSubobject<UFINLog>("Log");
	SetReplicates(true);
	bReplicateUsingRegisteredSubObjectList = true;
	AddReplicatedSubObject(Log);

	mFactoryTickFunction.bCanEverTick = true;
	mFactoryTickFunction.bStartWithTickEnabled = true;
	mFactoryTickFunction.bRunOnAnyThread = true;
	mFactoryTickFunction.bAllowTickOnDedicatedServer = true;

	if (HasAuthority()) mFactoryTickFunction.SetTickFunctionEnable(true);

	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AFINComputerCase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerCase, DataStorage);
	DOREPLIFETIME(AFINComputerCase, LastTabIndex);
	DOREPLIFETIME(AFINComputerCase, Log);
	DOREPLIFETIME(AFINComputerCase, InternalKernelState);
	DOREPLIFETIME(AFINComputerCase, Processors);
	DOREPLIFETIME(AFINComputerCase, PCIDevices);
	DOREPLIFETIME(AFINComputerCase, NetworkConnector);
}

void AFINComputerCase::OnConstruction(const FTransform& transform) {
	Super::OnConstruction(transform);
	
	Kernel = NewObject<UFINKernelSystem>(this, "Kernel");
	NetworkController = NewObject<UFINKernelNetworkController>(this, "NetworkController");
	AudioController = NewObject<UFINKernelAudioController>(this, "AudioController");

	Kernel->SetLog(Log);
	NetworkController->SetComponent(NetworkConnector);
	Kernel->SetNetwork(NetworkController);
	AudioController->SetComponent(Speaker);
	Kernel->SetAudio(AudioController);
}

void AFINComputerCase::BeginPlay() {
	Super::BeginPlay();

	if (HasAuthority()) {
		DataStorage->Resize(2);

		// load floppy
		FInventoryStack stack;
		if (DataStorage->GetStackFromIndex(1, stack)) {
			const TSubclassOf<UFINComputerDriveDesc> DriveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.GetItemClass());
			AFINFileSystemState* FileSystemState = Cast<AFINFileSystemState>(stack.Item.ItemState.Get());
			if (IsValid(DriveDesc)) {
				if (!IsValid(FileSystemState)) {
					FileSystemState = AFINFileSystemState::CreateState(this, UFINComputerDriveDesc::GetStorageCapacity(DriveDesc), DataStorage, 1);
				}
			}
			if (Floppy) Kernel->RemoveDrive(Floppy);
			Floppy = FileSystemState;
			if (Floppy) Kernel->AddDrive(Floppy);
		}
	}
}

void AFINComputerCase::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	if (HasAuthority()) {
		Log->Tick();
		
		if (Kernel) {
			Kernel->HandleFutures();
			if (Kernel->GetState() != InternalKernelState) {
				netSig_ComputerStateChanged(InternalKernelState, Kernel->GetState());
				InternalKernelState = Kernel->GetState();
			}
		}
	}
}

void AFINComputerCase::Factory_Tick(float dt) {
	if (HasAuthority() && Kernel) {
		KernelTickTime += dt;
		if (KernelTickTime > 10.0) KernelTickTime = 10.0;

		float KernelTicksPerSec = 1.0;
		if (Processors.Num() >= 1) KernelTicksPerSec = Processors[0]->KernelTicksPerSecond;

		while (KernelTickTime > 1.0/KernelTicksPerSec) {
			KernelTickTime -= 1.0/KernelTicksPerSec;
			Kernel->Tick(dt);
		}
	}
}

bool AFINComputerCase::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerCase::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	//out_dependentObjects.Add(AFINComputerSubsystem::GetComputerSubsystem(this)); // TODO: Check if really not needed anymore
}

void AFINComputerCase::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	TArray<AActor*> modules;
	Panel->GetModules(modules);
	AddModules(modules);
}

void AFINComputerCase::NetMulti_OnEEPROMChanged_Implementation(AFINStateEEPROM* EEPROM) {
	OnEEPROMUpdate.Broadcast(EEPROM);
}

void AFINComputerCase::NetMulti_OnFloppyChanged_Implementation(AFINFileSystemState* NewFloppy) {
	OnFloppyUpdate.Broadcast(NewFloppy);
}

void AFINComputerCase::AddProcessor(AFINComputerProcessor* processor) {
	Processors.Add(processor);
	ForceNetUpdate();
	if (Processors.Num() == 1) {
		// no processor already added -> add new processor
		UFINKernelProcessor* NewProcessor = processor->CreateProcessor();
		if (!Kernel->GetProcessor() || Kernel->GetProcessor()->GetClass() != NewProcessor->GetClass()) {
			Kernel->SetProcessor(NewProcessor);
		}
		Kernel->GetProcessor()->SetEEPROM(UFINComputerEEPROMDesc::GetEEPROM(DataStorage, 0));
		Kernel->GetProcessor()->DebugInfo = GetName();
	} else {
		// processor already added
		Kernel->SetProcessor(nullptr);
	}
	if (Processors.Num() > 0) Panel->AllowedModules.Remove(AFINComputerProcessor::StaticClass());
	else Panel->AllowedModules.Add(AFINComputerProcessor::StaticClass());
}

void AFINComputerCase::RemoveProcessor(AFINComputerProcessor* processor) {
	Processors.Remove(processor);
	ForceNetUpdate();
	if (Processors.Num() == 1) {
		// two processors were added -> add leaving processor to kernel
		Kernel->SetProcessor(Processors[0]->CreateProcessor());
	} else {
		// more than two processors were added or no processor remaining -> remove processor from kernel
		Kernel->SetProcessor(nullptr);
	}
	
	if (Processors.Num() > 0) Panel->AllowedModules.Remove(AFINComputerProcessor::StaticClass());
	else Panel->AllowedModules.Add(AFINComputerProcessor::StaticClass());
}

void AFINComputerCase::AddMemory(AFINComputerMemory* memory) {
	Memories.Add(memory);
	RecalculateMemory();
}

void AFINComputerCase::RemoveMemory(AFINComputerMemory* memory) {
	Memories.Remove(memory);
	RecalculateMemory();
}

void AFINComputerCase::RecalculateMemory() {
	int64 capacity = 0;
	for (AFINComputerMemory* memory : Memories) {
		capacity += memory->GetCapacity();
	}
	Kernel->SetCapacity(capacity);
}

void AFINComputerCase::AddDrive(AFINComputerDriveHolder* DriveHolder) {
	if (DriveHolders.Contains(DriveHolder)) return;
	DriveHolders.Add(DriveHolder);
	DriveHolder->OnLockedUpdate.AddDynamic(this, &AFINComputerCase::OnDriveUpdate);
	AFINFileSystemState* FileSystemState = DriveHolder->GetDrive();
	if (FileSystemState) Kernel->AddDrive(FileSystemState);
}

void AFINComputerCase::RemoveDrive(AFINComputerDriveHolder* DriveHolder) {
	if (DriveHolders.Remove(DriveHolder) <= 0) return;
	DriveHolder->OnLockedUpdate.RemoveDynamic(this, &AFINComputerCase::OnDriveUpdate);
	AFINFileSystemState* FileSystemState = DriveHolder->GetDrive();
	if (FileSystemState) Kernel->RemoveDrive(FileSystemState);
}

void AFINComputerCase::AddPCIDevice(TScriptInterface<IFINPciDeviceInterface> InPCIDevice) {
	if (IFINPciDeviceInterface::Execute_NeedsPCINetworkConnection(InPCIDevice.GetObject())) {
		IFINPciDeviceInterface::Execute_SetPCINetworkConnection(InPCIDevice.GetObject(), NetworkConnector);
		NetworkConnector->AddConnectedNode(InPCIDevice.GetObject());
	}
	PCIDevices.Add(InPCIDevice.GetObject());
	Kernel->AddPCIDevice(InPCIDevice);
}

void AFINComputerCase::RemovePCIDevice(TScriptInterface<IFINPciDeviceInterface> InPCIDevice) {
	if (IFINPciDeviceInterface::Execute_NeedsPCINetworkConnection(InPCIDevice.GetObject())) {
		IFINPciDeviceInterface::Execute_SetPCINetworkConnection(InPCIDevice.GetObject(), nullptr);
		NetworkConnector->RemoveConnectedNode(InPCIDevice.GetObject());
	}
	PCIDevices.Remove(InPCIDevice.GetObject());
	Kernel->RemovePCIDevice(InPCIDevice);
}

void AFINComputerCase::AddModule(AActor* module) {
	if (HasAuthority()) {
		if (AFINComputerProcessor* processor = Cast<AFINComputerProcessor>(module)) {
			AddProcessor(processor);
		} else if (AFINComputerMemory* memory = Cast<AFINComputerMemory>(module)) {
			AddMemory(memory);
		} else if (AFINComputerDriveHolder* holder = Cast<AFINComputerDriveHolder>(module)) {
			AddDrive(holder);
		} else if (module->Implements<UFINPciDeviceInterface>()) {
			AddPCIDevice(module);
		}
	}
}

void AFINComputerCase::RemoveModule(AActor* module) {
	if (HasAuthority()) {
		if (AFINComputerProcessor* processor = Cast<AFINComputerProcessor>(module)) {
			RemoveProcessor(processor);
		} else if (AFINComputerMemory* memory = Cast<AFINComputerMemory>(module)) {
			RemoveMemory(memory);
		} else if (AFINComputerDriveHolder* holder = Cast<AFINComputerDriveHolder>(module)) {
			RemoveDrive(holder);
		} else if (module->Implements<UFINPciDeviceInterface>()) {
			RemovePCIDevice(module);
		}
	}
}

void AFINComputerCase::AddModules(const TArray<AActor*>& Modules) {
	for (AActor* Module : Modules) {
		AddModule(Module);
	}
}

void AFINComputerCase::OnModuleChanged(UObject* module, bool added) {
	if (HasAuthority() && module->Implements<UFINModuleSystemModule>()) {
		AActor* moduleActor = Cast<AActor>(module);
		if (added) AddModule(moduleActor);
		else RemoveModule(moduleActor);
	}
}

void AFINComputerCase::OnEEPROMChanged(TSubclassOf<UFGItemDescriptor> Item, int32 Num) {
	if (HasAuthority() && Kernel) {
		if (Item->IsChildOf<UFINComputerEEPROMDesc>()) {
			UFINKernelProcessor* Processor = Kernel->GetProcessor();
			AFINStateEEPROM* EEPROM = UFINComputerEEPROMDesc::GetEEPROM(DataStorage, 0);
			if (Processor) Processor->SetEEPROM(EEPROM);
			NetMulti_OnEEPROMChanged(EEPROM);
		} else if (Item->IsChildOf<UFINComputerDriveDesc>()) {
			AFINFileSystemState* state = nullptr;
			FInventoryStack stack;
			if (DataStorage->GetStackFromIndex(1, stack)) {
				const TSubclassOf<UFINComputerDriveDesc> DriveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.GetItemClass());
				state = Cast<AFINFileSystemState>(stack.Item.ItemState.Get());
				if (IsValid(DriveDesc)) {
					if (!IsValid(state)) {
						state = AFINFileSystemState::CreateState(this, UFINComputerDriveDesc::GetStorageCapacity(DriveDesc), DataStorage, 1);
					}
				}
			}
			if (IsValid(Floppy)) {
				Kernel->RemoveDrive(Floppy);
				Floppy = nullptr;
			}
			if (IsValid(state)) {
				Floppy = state;
				Kernel->AddDrive(Floppy);
			}
			NetMulti_OnFloppyChanged(Floppy);
		}
	}
}

void AFINComputerCase::Toggle() {
	if (HasAuthority()) {
		UFINKernelProcessor* Processor = Kernel->GetProcessor();
		if (Processor) Processor->SetEEPROM(UFINComputerEEPROMDesc::GetEEPROM(DataStorage, 0));
		switch (Kernel->GetState()) {
		case FIN_KERNEL_SHUTOFF:
			Log->EmptyLog();
			Kernel->Start(false);
			break;
		case FIN_KERNEL_CRASHED:
			Log->EmptyLog();
			Kernel->Start(true);
			break;
		default:
			Kernel->Stop();	
			break;
		}
	}
}

FString AFINComputerCase::GetCrash() {
	if (!Kernel->GetCrash().IsValid()) return "";
	return Kernel->GetCrash()->Message;
}

EFINKernelState AFINComputerCase::GetState() {
	return InternalKernelState;
}

AFINComputerProcessor* AFINComputerCase::GetProcessor() {
	if (Processors.Num() != 1) return nullptr;
	return Processors[0];
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFINComputerCase::HandleSignal(const FFINSignalData& signal, const FFINNetworkTrace& sender) {
	if (Kernel) Kernel->GetNetwork()->PushSignal(signal, sender);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFINComputerCase::OnDriveUpdate(bool bOldLocked, AFINFileSystemState* drive) {
	if (bOldLocked) {
		Kernel->RemoveDrive(drive);
	} else {
		Kernel->AddDrive(drive);
	}
}

void AFINComputerCase::netSig_ComputerStateChanged_Implementation(int64 PrevState, int64 NewState) {}

void AFINComputerCase::netSig_FileSystemUpdate_Implementation(int Type, const FString& From, const FString& To) {}

int64 AFINComputerCase::netFunc_getState() {
	return InternalKernelState;
}

void AFINComputerCase::netFunc_startComputer() {
	if (GetState() != EFINKernelState::FIN_KERNEL_RUNNING) Log->EmptyLog();
	Kernel->Start(false);
}

void AFINComputerCase::netFunc_stopComputer() {
	Kernel->Stop();
}

void AFINComputerCase::netFunc_getLog(int64 PageSize, int64 Page, TArray<FFINLogEntry>& OutLog, int64& OutLogSize) {
	FScopeLock Lock = Log->Lock();
	const TArray<FFINLogEntry>& Entries = Log->GetLogEntries();
	OutLog = UFINUtils::PaginateArray(TArrayView<const FFINLogEntry>(Entries), PageSize, Page);
	if (Page < 0) Algo::Reverse(OutLog);
	OutLogSize = Entries.Num();
}
