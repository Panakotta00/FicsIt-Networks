#include "FINComputerCase.h"

#include "FicsItNetworksCustomVersion.h"
#include "FINComputerProcessor.h"
#include "FINComputerMemory.h"
#include "FINComputerDriveHolder.h"
#include "FicsItKernel/Processor/Lua/LuaProcessor.h"
#include "FGInventoryComponent.h"
#include "FINComputerDriveDesc.h"
#include "FINComputerEEPROMDesc.h"
#include "FINComputerFloppyDesc.h"
#include "FINComputerNetworkCard.h"
#include "FINComputerSubsystem.h"
#include "UnrealNetwork.h"
#include "FicsItKernel/FicsItKernel.h"

AFINComputerCase::AFINComputerCase() {
	NetworkConnector = CreateDefaultSubobject<UFINAdvancedNetworkConnectionComponent>("NetworkConnector");
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->OnNetworkSignal.AddDynamic(this, &AFINComputerCase::HandleSignal);
	
	Panel = CreateDefaultSubobject<UFINModuleSystemPanel>("Panel");
	Panel->SetupAttachment(RootComponent);
	Panel->OnModuleChanged.AddDynamic(this, &AFINComputerCase::OnModuleChanged);

	DataStorage = CreateDefaultSubobject<UFGInventoryComponent>("DataStorage");
	DataStorage->OnItemRemovedDelegate.AddDynamic(this, &AFINComputerCase::OnEEPROMChanged);
	DataStorage->OnItemAddedDelegate.AddDynamic(this, &AFINComputerCase::OnEEPROMChanged);
	DataStorage->mItemFilter.BindLambda([](TSubclassOf<UObject> item, int32 i) {
        return (i == 0 && item->IsChildOf<UFINComputerEEPROMDesc>()) || (i == 1 && item->IsChildOf<UFINComputerFloppyDesc>());
    });
	DataStorage->SetIsReplicated(true);

	Speaker = CreateDefaultSubobject<UAudioComponent>("Speaker");
	Speaker->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

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
	DOREPLIFETIME(AFINComputerCase, SerialOutput);
	DOREPLIFETIME(AFINComputerCase, Screens);
	DOREPLIFETIME(AFINComputerCase, InternalKernelState);
	DOREPLIFETIME(AFINComputerCase, Processors);
}

void AFINComputerCase::OnConstruction(const FTransform& transform) {
	Super::OnConstruction(transform);
	
	Kernel = NewObject<UFINKernelSystem>(this, "Kernel");
	NetworkController = NewObject<UFINKernelNetworkController>(this, "NetworkController");
	AudioController = NewObject<UFINKernelAudioController>(this, "AudioController");

	NetworkController->SetComponent(NetworkConnector);
	Kernel->SetNetwork(NetworkController);
	AudioController->SetComponent(Speaker);
	Kernel->SetAudio(AudioController);
}

#pragma optimize("", off)
void AFINComputerCase::BeginPlay() {
	Super::BeginPlay();

	if (HasAuthority()) {
		DataStorage->Resize(2);

		// load floppy
		FInventoryStack stack;
		if (DataStorage->GetStackFromIndex(1, stack)) {
			const TSubclassOf<UFINComputerDriveDesc> DriveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.ItemClass);
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
#pragma optimize("", on)

void AFINComputerCase::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	
	if (HasAuthority()) {
		bool bNetUpdate = false;
		if (Kernel) {
			Kernel->HandleFutures();
			if (Kernel->GetState() != InternalKernelState) {
				InternalKernelState = Kernel->GetState();
				bNetUpdate = true;
			}
		}
		if (OldSerialOutput != SerialOutput) {
			OldSerialOutput = SerialOutput;
			bNetUpdate = true;
		}
		if (bNetUpdate) {
			ForceNetUpdate();
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
			//auto n = std::chrono::high_resolution_clock::now();
			Kernel->Tick(dt);
			//auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - n);
			//SML::Logging::debug("Computer tick: ", dur.count());
		
			FileSystem::SRef<FFINKernelFSDevDevice> DevDevice = Kernel->GetDevDevice();
			if (DevDevice && DevDevice->getSerial()) {
				SerialOutput = SerialOutput.Append(UTF8_TO_TCHAR(DevDevice->getSerial()->readOutput().c_str()));
				SerialOutput = SerialOutput.Right(1000);
			}
		}
	}
}

bool AFINComputerCase::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerCase::GatherDependencies_Implementation(TArray<UObject*>& out_dependentObjects) {
	out_dependentObjects.Add(AFINComputerSubsystem::GetComputerSubsystem(this));
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

void AFINComputerCase::AddGPU(AFINComputerGPU* GPU) {
	Kernel->AddGPU(GPU);
}

void AFINComputerCase::RemoveGPU(AFINComputerGPU* GPU) {
	Kernel->RemoveGPU(GPU);
}

void AFINComputerCase::AddScreen(AFINComputerScreen* Screen) {
	Kernel->AddScreen(Screen);
	Screens.Add(Screen);
}

void AFINComputerCase::RemoveScreen(AFINComputerScreen* Screen) {
	Kernel->RemoveScreen(Screen);
	Screens.Remove(Screen);
}

void AFINComputerCase::AddNetCard(AFINComputerNetworkCard* NetCard) {
	NetCard->ConnectedComponent = NetworkConnector;
	NetworkConnector->AddConnectedNode(NetCard);
	NetworkCards.Add(NetCard);
}

void AFINComputerCase::RemoveNetCard(AFINComputerNetworkCard* NetCard) {
	NetCard->ConnectedComponent = nullptr;
	NetworkConnector->RemoveConnectedNode(NetCard);
	NetworkCards.Remove(NetCard);
}

void AFINComputerCase::AddModule(AActor* module) {
	if (HasAuthority()) {
		if (AFINComputerProcessor* processor = Cast<AFINComputerProcessor>(module)) {
			AddProcessor(processor);
		} else if (AFINComputerMemory* memory = Cast<AFINComputerMemory>(module)) {
			AddMemory(memory);
		} else if (AFINComputerDriveHolder* holder = Cast<AFINComputerDriveHolder>(module)) {
			AddDrive(holder);
		} else if (AFINComputerScreen* screen = Cast<AFINComputerScreen>(module)) {
			AddScreen(screen);
		} else if (AFINComputerGPU* gpu = Cast<AFINComputerGPU>(module)) {
			AddGPU(gpu);
		} else if (AFINComputerNetworkCard* netCard = Cast<AFINComputerNetworkCard>(module)) {
			AddNetCard(netCard);
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
		} else if (AFINComputerScreen* screen = Cast<AFINComputerScreen>(module)) {
			RemoveScreen(screen);
		} else if (AFINComputerGPU* gpu = Cast<AFINComputerGPU>(module)) {
			RemoveGPU(gpu);
		} else if (AFINComputerNetworkCard* netCard = Cast<AFINComputerNetworkCard>(module)) {
			RemoveNetCard(netCard);
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
	if (HasAuthority()) {
		if (Item->IsChildOf<UFINComputerEEPROMDesc>()) {
			UFINKernelProcessor* Processor = Kernel->GetProcessor();
			AFINStateEEPROM* EEPROM = UFINComputerEEPROMDesc::GetEEPROM(DataStorage, 0);
			if (Processor) Processor->SetEEPROM(EEPROM);
			NetMulti_OnEEPROMChanged(EEPROM);
		} else if (Item->IsChildOf<UFINComputerDriveDesc>()) {
			AFINFileSystemState* state = nullptr;
			FInventoryStack stack;
			if (DataStorage->GetStackFromIndex(1, stack)) {
				const TSubclassOf<UFINComputerDriveDesc> DriveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.ItemClass);
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
			Kernel->Start(false);
			SerialOutput = "";
			ForceNetUpdate();
			break;
		case FIN_KERNEL_CRASHED:
			Kernel->Start(true);
			SerialOutput = "";
			ForceNetUpdate();
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

FString AFINComputerCase::GetSerialOutput() {
	return SerialOutput;
}

AFINComputerProcessor* AFINComputerCase::GetProcessor() {
	if (Processors.Num() != 1) return nullptr;
	return Processors[0];
}

void AFINComputerCase::WriteSerialInput(const FString& str) {
	FileSystem::SRef<FFINKernelFSDevDevice> DevDevice = Kernel->GetDevDevice();
	if (DevDevice) {
		DevDevice->getSerial()->write(TCHAR_TO_UTF8(*str));
	}
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
