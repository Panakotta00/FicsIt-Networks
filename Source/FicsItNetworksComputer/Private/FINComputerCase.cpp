#include "FINComputerCase.h"

#include "FicsItKernel/FicsItKernel.h"
#include "FGInventoryComponent.h"
#include "FGPlayerController.h"
#include "FILLogContainer.h"
#include "FINAdvancedNetworkConnectionComponent.h"
#include "FINComputerEEPROMDesc.h"
#include "FINComputerFloppyDesc.h"
#include "FINComputerRCO.h"
#include "FINFileSystemSubsystem.h"
#include "FINUtils.h"
#include "Components/AudioComponent.h"
#include "ComputerModules/FINComputerDriveHolder.h"
#include "ComputerModules/FINComputerMemory.h"
#include "ComputerModules/FINComputerProcessor.h"
#include "FicsItKernel/FicsItFS/FINItemStateFileSystem.h"
#include "FicsItKernel/Network/NetworkController.h"
#include "FicsItKernel/Processor/Processor.h"
#include "ModuleSystem/FINModuleSystemPanel.h"
#include "Net/UnrealNetwork.h"

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

	Log = CreateDefaultSubobject<UFILLogContainer>("Log");
	bReplicates = true;
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

	if (HasAuthority()) {
		Kernel = NewObject<UFINKernelSystem>(this, "Kernel");
		NetworkController = NewObject<UFINKernelNetworkController>(this, "NetworkController");
		AudioController = NewObject<UFINKernelAudioController>(this, "AudioController");

		Kernel->SetLog(Log);
		NetworkController->SetComponent(NetworkConnector);
		Kernel->SetNetwork(NetworkController);
		AudioController->SetComponent(Speaker);
		Kernel->SetAudio(AudioController);
		Kernel->OnGetEEPROM.BindWeakLambda(this, [this]() {
			FInventoryStack stack;
			DataStorage->GetStackFromIndex(0, stack);
			return stack.Item;
		});
		Kernel->OnSetEEPROM.BindWeakLambda(this, [this](const FFGDynamicStruct& state) {
			FInventoryStack stack;
			if (DataStorage->GetStackFromIndex(0, stack) || stack.HasItems()) {
				DataStorage->SetStateOnIndex(0, state);
				GetWorld()->GetFirstPlayerController<AFGPlayerController>()->GetRemoteCallObjectOfClass<UFINComputerRCO>()->Multicast_ItemStateUpdated(DataStorage, stack.Item.GetItemClass());
				return true;
			}
			return false;
		});
	}
}

void AFINComputerCase::BeginPlay() {
	Super::BeginPlay();

	if (HasAuthority()) {
		DataStorage->Resize(2);

		// load floppy
		FInventoryStack stack;
		if (DataStorage->GetStackFromIndex(1, stack)) {
			const TSubclassOf<UFINComputerDriveDesc> DriveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.GetItemClass());
			auto FileSystemState = stack.Item.GetItemState().GetValuePtr<FFINItemStateFileSystem>();
			FGuid fileSystemID;
			if (IsValid(DriveDesc)) {
				if (FileSystemState) {
					fileSystemID = FileSystemState->ID;
				} else {
					fileSystemID = AFINFileSystemSubsystem::CreateState(UFINComputerDriveDesc::GetStorageCapacity(DriveDesc), DataStorage, 1);
				}
			}
			if (Floppy.IsValid()) Kernel->RemoveDrive(Floppy);
			Floppy = fileSystemID;
			if (Floppy.IsValid()) Kernel->AddDrive(Floppy);
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

void AFINComputerCase::NetMulti_OnEEPROMChanged_Implementation(const FFGDynamicStruct& EEPROM) {
	OnEEPROMUpdate.Broadcast(EEPROM);
}

void AFINComputerCase::NetMulti_OnFloppyChanged_Implementation(const FGuid& NewFloppy) {
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
	FGuid FileSystemID = DriveHolder->GetDrive();
	if (FileSystemID.IsValid()) Kernel->AddDrive(FileSystemID);
}

void AFINComputerCase::RemoveDrive(AFINComputerDriveHolder* DriveHolder) {
	if (DriveHolders.Remove(DriveHolder) <= 0) return;
	DriveHolder->OnLockedUpdate.RemoveDynamic(this, &AFINComputerCase::OnDriveUpdate);
	FGuid FileSystemID = DriveHolder->GetDrive();
	if (FileSystemID.IsValid()) Kernel->RemoveDrive(FileSystemID);
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

void AFINComputerCase::OnEEPROMChanged(TSubclassOf<UFGItemDescriptor> Item, int32 Num, UFGInventoryComponent* changedInventory) {
	if (HasAuthority() && Kernel) {
		if (Item->IsChildOf<UFINComputerEEPROMDesc>()) {
			NetMulti_OnEEPROMChanged(GetEEPROM());
		} else if (Item->IsChildOf<UFINComputerDriveDesc>()) {
			FInventoryStack stack;
			FGuid fileSystemID;
			if (DataStorage->GetStackFromIndex(1, stack)) {
				const TSubclassOf<UFINComputerDriveDesc> DriveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.GetItemClass());
				auto state = stack.Item.GetItemState().GetValuePtr<FFINItemStateFileSystem>();
				if (IsValid(DriveDesc)) {
					if (state) {
						fileSystemID = state->ID;
					} else {
						fileSystemID = AFINFileSystemSubsystem::CreateState(UFINComputerDriveDesc::GetStorageCapacity(DriveDesc), DataStorage, 1);
					}
				}
			}
			if (Floppy.IsValid()) {
				Kernel->RemoveDrive(Floppy);
				Floppy = FGuid();
			}
			if (fileSystemID.IsValid()) {
				Floppy = fileSystemID;
				Kernel->AddDrive(Floppy);
			}
			NetMulti_OnFloppyChanged(Floppy);
		}
	}
}

FFGDynamicStruct AFINComputerCase::GetEEPROM() {
	return UFINComputerEEPROMDesc::GetEEPROM(DataStorage, 0);
}

void AFINComputerCase::Toggle() {
	if (HasAuthority()) {
		UFINKernelProcessor* Processor = Kernel->GetProcessor();
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
void AFINComputerCase::HandleSignal(const FFINSignalData& signal, const FFIRTrace& sender) {
	if (Kernel) Kernel->GetNetwork()->PushSignal(signal, sender);
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AFINComputerCase::OnDriveUpdate(bool bOldLocked, const FGuid& drive) {
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

void AFINComputerCase::netFunc_getLog(int64 PageSize, int64 Page, TArray<FFILEntry>& OutLog, int64& OutLogSize) {
	FScopeLock Lock = Log->Lock();
	const TArray<FFILEntry>& Entries = Log->GetLogEntries();
	OutLog = UFINUtils::PaginateArray(TArrayView<const FFILEntry>(Entries), PageSize, Page);
	if (Page < 0) Algo::Reverse(OutLog);
	OutLogSize = Entries.Num();
}
