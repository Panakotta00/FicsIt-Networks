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
#include "FINConfig.h"
#include "UnrealNetwork.h"
#include "FicsItKernel/FicsItKernel.h"
#include "FicsItKernel/Audio/AudioComponentController.h"
#include "util/Logging.h"

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

	SpeakerTrampoline = CreateDefaultSubobject<UFINAudioComponentControllerTrampoline>("SpeakerTrampoline");
	SpeakerTrampoline->Speaker = Speaker;
	
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

AFINComputerCase::~AFINComputerCase() {
	if (kernel) delete kernel;
}

void AFINComputerCase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFINComputerCase, DataStorage);
	DOREPLIFETIME(AFINComputerCase, LastTabIndex);
	DOREPLIFETIME(AFINComputerCase, SerialOutput);
	DOREPLIFETIME(AFINComputerCase, Screens);
	DOREPLIFETIME(AFINComputerCase, InternalKernelState);
}

void AFINComputerCase::Serialize(FArchive& Ar) {
	Super::Serialize(Ar);
	if (Ar.IsSaveGame() && AFINComputerSubsystem::GetComputerSubsystem(this)->Version >= EFINCustomVersion::FINSignalStorage) {
		kernel->Serialize(Ar, KernelState);
	}
}

void AFINComputerCase::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	
	kernel = new FicsItKernel::KernelSystem();
	kernel->setNetwork(new FicsItKernel::Network::NetworkController());
	kernel->getNetwork()->component = NetworkConnector;
	if (finConfig->HasField("SignalQueueSize")) kernel->getNetwork()->maxSignalCount = finConfig->GetIntegerField("SignalQueueSize");
	kernel->setAudio(new FicsItKernel::Audio::AudioComponentController(SpeakerTrampoline));
}

void AFINComputerCase::BeginPlay() {
	Super::BeginPlay();

	if (HasAuthority()) {
		DataStorage->Resize(2);

		// load floppy
		AFINFileSystemState* state = nullptr;
		FInventoryStack stack;
		if (DataStorage->GetStackFromIndex(1, stack)) {
			TSubclassOf<UFINComputerDriveDesc> DriveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.ItemClass);
			state = Cast<AFINFileSystemState>(stack.Item.ItemState.Get());
			if (IsValid(DriveDesc)) {
				if (!IsValid(state)) {
					state = AFINFileSystemState::CreateState(this, UFINComputerDriveDesc::GetStorageCapacity(DriveDesc), DataStorage, 1);
				}
			}
			if (Floppy) kernel->removeDrive(Floppy);
			Floppy = state;
			if (Floppy) kernel->addDrive(Floppy);
		}
	}
}

void AFINComputerCase::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) {
	if (HasAuthority()) {
		bool bNetUpdate = false;
		if (kernel) {
			kernel->handleFutures();
			EComputerState NewState;
			using State = FicsItKernel::KernelState;
			switch (kernel->getState()) {
			case State::RUNNING:
				NewState = EComputerState::RUNNING;
				break;
			case State::SHUTOFF:
				NewState = EComputerState::SHUTOFF;
				break;
			default:
				NewState = EComputerState::CRASHED;
				break;
			}
			if (NewState != InternalKernelState) {
				InternalKernelState = NewState;
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
	if (kernel) {
		KernelTickTime += dt;
		if (KernelTickTime > 10.0) KernelTickTime = 10.0;

		float KernelTicksPerSec = 1.0;
		if (Processors.Num() >= 1) KernelTicksPerSec = Processors.begin().ElementIt->Value->KernelTicksPerSecond;

		while (KernelTickTime > 1.0/KernelTicksPerSec) {
			KernelTickTime -= 1.0/KernelTicksPerSec;
			//auto n = std::chrono::high_resolution_clock::now();
			kernel->tick(dt);
			//auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - n);
			//SML::Logging::debug("Computer tick: ", dur.count());
		
			FileSystem::SRef<FicsItKernel::FicsItFS::DevDevice> dev = kernel->getDevDevice();
			if (dev && dev->getSerial()) {
				SerialOutput = SerialOutput.Append(UTF8_TO_TCHAR(dev->getSerial()->readOutput().c_str()));
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

void AFINComputerCase::PreLoadGame_Implementation(int32 gameVersion, int32 engineVersion) {
	kernel->PreSerialize(KernelState, true);
}

void AFINComputerCase::PostLoadGame_Implementation(int32 saveVersion, int32 gameVersion) {
	TArray<AActor*> modules;
	Panel->GetModules(modules);
	AddModules(modules);
	
	kernel->PostSerialize(KernelState, true);
}

void AFINComputerCase::PreSaveGame_Implementation(int32 gameVersion, int32 engineVersion) {
	kernel->PreSerialize(KernelState, false);
}

void AFINComputerCase::PostSaveGame_Implementation(int32 gameVersion, int32 engineVersion) {
	kernel->PostSerialize(KernelState, false);
}

void AFINComputerCase::NetMulti_OnEEPROMChanged_Implementation(AFINStateEEPROM* EEPROM) {
	OnEEPROMUpdate.Broadcast(EEPROM);
}

void AFINComputerCase::NetMulti_OnFloppyChanged_Implementation(AFINFileSystemState* NewFloppy) {
	OnFloppyUpdate.Broadcast(NewFloppy);
}

void AFINComputerCase::AddProcessor(AFINComputerProcessor* processor) {
	Processors.Add(processor);
	if (Processors.Num() == 1) {
		// no processor already added -> add new processor
		kernel->setProcessor(processor->CreateProcessor());
		kernel->getProcessor()->setEEPROM(UFINComputerEEPROMDesc::GetEEPROM(DataStorage, 0));
	} else {
		// processor already added
		kernel->setProcessor(nullptr);
	}
	if (Processors.Num() > 0) Panel->AllowedModules.Remove(AFINComputerProcessor::StaticClass());
	else Panel->AllowedModules.Add(AFINComputerProcessor::StaticClass());
}

void AFINComputerCase::RemoveProcessor(AFINComputerProcessor* processor) {
	Processors.Remove(processor);
	if (Processors.Num() == 1) {
		// two processors were added -> add leaving processor to kernel
		kernel->setProcessor((*Processors.Find(0))->CreateProcessor());
	} else {
		// more than two processors were added or no processor remaining -> remove processor from kernel
		kernel->setProcessor(nullptr);
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
	kernel->setCapacity(capacity);
}

void AFINComputerCase::AddDrive(AFINComputerDriveHolder* DriveHolder) {
	if (DriveHolders.Contains(DriveHolder)) return;
	DriveHolders.Add(DriveHolder);
	DriveHolder->OnLockedUpdate.AddDynamic(this, &AFINComputerCase::OnDriveUpdate);
	AFINFileSystemState* FileSystemState = DriveHolder->GetDrive();
	if (FileSystemState) kernel->addDrive(FileSystemState);
}

void AFINComputerCase::RemoveDrive(AFINComputerDriveHolder* DriveHolder) {
	if (DriveHolders.Remove(DriveHolder) <= 0) return;
	DriveHolder->OnLockedUpdate.RemoveDynamic(this, &AFINComputerCase::OnDriveUpdate);
	AFINFileSystemState* FileSystemState = DriveHolder->GetDrive();
	if (FileSystemState) kernel->removeDrive(FileSystemState);
}

void AFINComputerCase::AddGPU(AFINComputerGPU* GPU) {
	kernel->addGPU(GPU);
}

void AFINComputerCase::RemoveGPU(AFINComputerGPU* GPU) {
	kernel->removeGPU(GPU);
}

void AFINComputerCase::AddScreen(AFINComputerScreen* Screen) {
	kernel->addScreen(Screen);
	Screens.Add(Screen);
}

void AFINComputerCase::RemoveScreen(AFINComputerScreen* Screen) {
	kernel->removeScreen(Screen);
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
			FicsItKernel::Processor* processor = kernel->getProcessor();
			AFINStateEEPROM* EEPROM = UFINComputerEEPROMDesc::GetEEPROM(DataStorage, 0);
			if (processor) processor->setEEPROM(EEPROM);
			NetMulti_OnEEPROMChanged(EEPROM);
		} else if (Item->IsChildOf<UFINComputerDriveDesc>()) {
			AFINFileSystemState* state = nullptr;
			FInventoryStack stack;
			if (DataStorage->GetStackFromIndex(1, stack)) {
				TSubclassOf<UFINComputerDriveDesc> driveDesc = TSubclassOf<UFINComputerDriveDesc>(stack.Item.ItemClass);
				state = Cast<AFINFileSystemState>(stack.Item.ItemState.Get());
				if (IsValid(driveDesc)) {
					if (!IsValid(state)) {
						state = AFINFileSystemState::CreateState(this, UFINComputerDriveDesc::GetStorageCapacity(driveDesc), DataStorage, 1);
					}
				}
			}
			if (IsValid(Floppy)) {
				kernel->removeDrive(Floppy);
				Floppy = nullptr;
			}
			if (IsValid(state)) {
				Floppy = state;
				kernel->addDrive(Floppy);
			}
			NetMulti_OnFloppyChanged(Floppy);
		}
	}
}

void AFINComputerCase::Toggle() {
	if (HasAuthority()) {
		FicsItKernel::Processor* processor = kernel->getProcessor();
		if (processor) processor->setEEPROM(UFINComputerEEPROMDesc::GetEEPROM(DataStorage, 0));
		switch (kernel->getState()) {
		case FicsItKernel::KernelState::SHUTOFF:
			kernel->start(false);
			SerialOutput = "";
			ForceNetUpdate();
			break;
		case FicsItKernel::KernelState::CRASHED:
			kernel->start(true);
			SerialOutput = "";
			ForceNetUpdate();
			break;
		default:
			kernel->stop();	
			break;
		}
	}
}

FString AFINComputerCase::GetCrash() {
	return UTF8_TO_TCHAR(kernel->getCrash().what());
}

EComputerState AFINComputerCase::GetState() {
	return InternalKernelState;
}

FString AFINComputerCase::GetSerialOutput() {
	return SerialOutput;
}

void AFINComputerCase::WriteSerialInput(const FString& str) {
	FileSystem::SRef<FicsItKernel::FicsItFS::DevDevice> dev = kernel->getDevDevice();
	if (dev) {
		dev->getSerial()->write(TCHAR_TO_UTF8(*str));
	}
}

void AFINComputerCase::HandleSignal(const FFINDynamicStructHolder& signal, const FFINNetworkTrace& sender) {
	if (kernel) kernel->getNetwork()->pushSignal(signal, sender);
}

void AFINComputerCase::OnDriveUpdate(bool bOldLocked, AFINFileSystemState* drive) {
	if (bOldLocked) {
		kernel->removeDrive(drive);
	} else {
		kernel->addDrive(drive);
	}
}
