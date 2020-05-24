#include "FINComputerCase.h"

#include "FicsItNetworksCustomVersion.h"
#include "FINComputerProcessor.h"
#include "FINComputerMemory.h"
#include "FINComputerDriveHolder.h"
#include "FicsItKernel/Processor/Lua/LuaProcessor.h"

AFINComputerCase::AFINComputerCase() {
	NetworkConnector = CreateDefaultSubobject<UFINNetworkConnector>("NetworkConnector");
	NetworkConnector->AddMerged(this);
	NetworkConnector->SetupAttachment(RootComponent);
	NetworkConnector->OnNetworkSignal.AddDynamic(this, &AFINComputerCase::HandleSignal);
	
	Panel = CreateDefaultSubobject<UFINModuleSystemPanel>("Panel");
	Panel->SetupAttachment(RootComponent);
	Panel->OnModuleChanged.AddDynamic(this, &AFINComputerCase::OnModuleChanged);

	mFactoryTickFunction.bCanEverTick = true;
	mFactoryTickFunction.bStartWithTickEnabled = true;
	mFactoryTickFunction.bRunOnAnyThread = true;
	mFactoryTickFunction.bAllowTickOnDedicatedServer = true;

	if (HasAuthority()) mFactoryTickFunction.SetTickFunctionEnable(true);

	kernel = new FicsItKernel::KernelSystem();
	kernel->setNetwork(new FicsItKernel::Network::NetworkController());
	kernel->getNetwork()->component = NetworkConnector;
}

AFINComputerCase::~AFINComputerCase() {
	if (kernel) delete kernel;
}

void AFINComputerCase::Serialize(FArchive& ar) {
	if (ar.IsSaveGame()) {
		ar.UsingCustomVersion(FFINCustomVersion::GUID);
		if (ar.CustomVer(FFINCustomVersion::GUID) >= FFINCustomVersion::KernelSystemPersistency) {
			ar << NetworkConnector;
			ar << Panel;
			ar << Code;
			kernel->Serialize(ar, KernelState);
		} else {
			Super::Serialize(ar);
		}
	} else {
		Super::Serialize(ar);
	}
}

void AFINComputerCase::BeginPlay() {
	Super::BeginPlay();
}

void AFINComputerCase::Factory_Tick(float dt) {
	kernel->tick(dt);
}

bool AFINComputerCase::ShouldSave_Implementation() const {
	return true;
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

void AFINComputerCase::AddProcessor(AFINComputerProcessor* processor) {
	Processors.Add(processor);
	if (Processors.Num() == 1) {
		// no processor already added -> add new processor
		kernel->setProcessor(processor->CreateProcessor());
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
	DriveHolder->OnDriveUpdate.AddDynamic(this, &AFINComputerCase::OnDriveUpdate);
	AFINFileSystemState* FileSystemState = DriveHolder->GetDrive();
	if (FileSystemState) kernel->addDrive(FileSystemState);
}

void AFINComputerCase::RemoveDrive(AFINComputerDriveHolder* DriveHolder) {
	if (DriveHolders.Remove(DriveHolder) <= 0) return;
	DriveHolder->OnDriveUpdate.RemoveDynamic(this, &AFINComputerCase::OnDriveUpdate);
	AFINFileSystemState* FileSystemState = DriveHolder->GetDrive();
	if (FileSystemState) kernel->removeDrive(FileSystemState);
}

void AFINComputerCase::AddModule(AActor* module) {
	if (AFINComputerProcessor* processor = Cast<AFINComputerProcessor>(module)) {
		AddProcessor(processor);
	} else if (AFINComputerMemory* memory = Cast<AFINComputerMemory>(module)) {
		AddMemory(memory);
	} else if (AFINComputerDriveHolder* holder = Cast<AFINComputerDriveHolder>(module)) {
		AddDrive(holder);
	}
}

void AFINComputerCase::RemoveModule(AActor* module) {
	if (AFINComputerProcessor* processor = Cast<AFINComputerProcessor>(module)) {
		RemoveProcessor(processor);
	} else if (AFINComputerMemory* memory = Cast<AFINComputerMemory>(module)) {
		RemoveMemory(memory);
	} else if (AFINComputerDriveHolder* holder = Cast<AFINComputerDriveHolder>(module)) {
		RemoveDrive(holder);
	}
}

void AFINComputerCase::AddModules(const TArray<AActor*>& Modules) {
	for (AActor* Module : Modules) {
		AddModule(Module);
	}
}

void AFINComputerCase::OnModuleChanged(UObject* module, bool added) {
	if (module->Implements<UFINModuleSystemModule>()) {
		AActor* moduleActor = Cast<AActor>(module);
		if (added) AddModule(moduleActor);
		else RemoveModule(moduleActor);
	}
}

void AFINComputerCase::Toggle() {
	FicsItKernel::Lua::LuaProcessor* processor = (FicsItKernel::Lua::LuaProcessor*)kernel->getProcessor();
	if (processor) processor->setCode(TCHAR_TO_UTF8(*Code));
	switch (kernel->getState()) {
	case FicsItKernel::KernelState::RUNNING:
		kernel->stop();	
		break;
	case FicsItKernel::KernelState::SHUTOFF:
		kernel->start(false);
		SerialOutput = "";
		break;
	case FicsItKernel::KernelState::CRASHED:
		kernel->start(true);
		SerialOutput = "";
		break;
	}
}

FString AFINComputerCase::GetCrash() {
	return UTF8_TO_TCHAR(kernel->getCrash().what());
}

EComputerState AFINComputerCase::GetState() {
	using State = FicsItKernel::KernelState;
	switch (kernel->getState()) {
	case State::RUNNING:
		return EComputerState::RUNNING;
	case State::SHUTOFF:
		return EComputerState::SHUTOFF;
	default:
		return EComputerState::CRASHED;
	}
}

FString AFINComputerCase::GetSerialOutput() {
	FileSystem::SRef<FicsItKernel::FicsItFS::DevDevice> dev = kernel->getDevDevice();
	if (dev) {
		SerialOutput = SerialOutput.Append(dev->getSerial()->readOutput().c_str());
		SerialOutput = SerialOutput.Right(1000);
	}
	return SerialOutput;
}

void AFINComputerCase::WriteSerialInput(const FString& str) {
	FileSystem::SRef<FicsItKernel::FicsItFS::DevDevice> dev = kernel->getDevDevice();
	if (dev) {
		dev->getSerial()->write(TCHAR_TO_UTF8(*str));
	}
}

void AFINComputerCase::HandleSignal(FFINSignal signal, FFINNetworkTrace sender) {
	if (kernel) kernel->getNetwork()->pushSignal(signal, sender);
}

void AFINComputerCase::OnDriveUpdate(bool added, AFINFileSystemState* drive) {
	if (added) {
		kernel->addDrive(drive);
	} else {
		kernel->removeDrive(drive);
	}
}
