#include "FINComputerCase.h"


#include "FicsItNetworksCustomVersion.h"
#include "FINComputerProcessor.h"
#include "FINComputerMemory.h"
#include "FINComputerDriveHolder.h"

#include "FicsItKernel/Processor/Lua/LuaProcessor.h"
#include "FicsItNetworks/FicsItNetworksCustomVersion.h"

AFINComputerCase::AFINComputerCase() {
	NetworkConnector = CreateDefaultSubobject<UFINNetworkConnector>("NetworkConnector");
	NetworkConnector->AddMerged(this);
	NetworkConnector->SetupAttachment(RootComponent);
	
	Panel = CreateDefaultSubobject<UFINModuleSystemPanel>("Panel");
	Panel->SetupAttachment(RootComponent);
	Panel->OnModuleChanged.AddDynamic(this, &AFINComputerCase::OnModuleChanged);

	mFactoryTickFunction.bCanEverTick = true;
	mFactoryTickFunction.bStartWithTickEnabled = true;
	mFactoryTickFunction.bRunOnAnyThread = true;
	mFactoryTickFunction.bAllowTickOnDedicatedServer = true;

	if (HasAuthority()) mFactoryTickFunction.SetTickFunctionEnable(true);

	kernel = new FicsItKernel::KernelSystem();
}

AFINComputerCase::~AFINComputerCase() {
	if (kernel) delete kernel;
}

void AFINComputerCase::Serialize(FArchive& ar) {
	ar.UsingCustomVersion(FFINCustomVersion::GUID);
	if (ar.IsSaveGame()) {
		if (ar.IsSaving()) {
			// save kernel persistent state
			State = "";
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&State);
			FJsonSerializer::Serialize(kernel->persist().ToSharedRef(), Writer);
		}
	}
	Super::Serialize(ar);
}

void AFINComputerCase::BeginPlay() {
	Super::BeginPlay();

	NetworkConnector->OnNetworkSignal.AddDynamic(this, &AFINComputerCase::HandleSignal);
	
	kernel->setNetwork(new FicsItKernel::Network::NetworkController());
	kernel->getNetwork()->component = NetworkConnector;

	recalculateKernelResources();

	// Recover saved state
	if (State.Len() > 0) {
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(State);
		TSharedPtr<FJsonObject> json;
		FJsonSerializer::Deserialize(Reader, json);
		kernel->unpersist(json);
	}
}

void AFINComputerCase::Factory_Tick(float dt) {
	kernel->tick(dt);
}

bool AFINComputerCase::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerCase::OnModuleChanged(UObject* module, bool added) {
	if (AFINComputerProcessor* processor = Cast<AFINComputerProcessor>(module)) {
		if (added) {
			if (kernel->getProcessor() == nullptr) {
				kernel->setProcessor(processor->CreateProcessor());
			} else {
				kernel->setProcessor(nullptr);
			}
			Panel->AllowedModules.Remove(AFINComputerProcessor::StaticClass());
		} else {
			if (kernel->getProcessor()) kernel->setProcessor(nullptr);
			else recalculateKernelResources();
			if (!kernel->getProcessor()) Panel->AllowedModules.Add(AFINComputerProcessor::StaticClass());
		}
	} else if (AFINComputerMemory* memory = Cast<AFINComputerMemory>(module)) {
		kernel->setCapacity(kernel->getCapacity() + ((added ? 1 : -1) * memory->GetCapacity()));
	} else if (AFINComputerDriveHolder* holder = Cast<AFINComputerDriveHolder>(module)) {
		holder->OnDriveUpdate.AddDynamic(this, &AFINComputerCase::OnDriveUpdate);
		AFINFileSystemState* state = holder->GetDrive();
		if (IsValid(state)) {
			if (added) kernel->addDrive(state);
			else kernel->removeDrive(state);
		}
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

void AFINComputerCase::recalculateKernelResources() {
	kernel->setCapacity(0);
	kernel->setProcessor(nullptr);
	TArray<AActor*> modules;
	Panel->GetModules(modules);
	for (AActor* module : modules) {
		OnModuleChanged(module, true);
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
