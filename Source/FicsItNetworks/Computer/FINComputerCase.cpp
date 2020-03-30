#include "FINComputerCase.h"

#include "FINComputerProcessor.h"
#include "FINComputerMemory.h"

#include "FicsItKernel/Processor/Lua/LuaProcessor.h"

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
}

AFINComputerCase::~AFINComputerCase() {
	if (kernel) delete kernel;
}

void AFINComputerCase::BeginPlay() {
	Super::BeginPlay();

	NetworkConnector->OnNetworkSignal.AddDynamic(this, &AFINComputerCase::HandleSignal);

	kernel = new FicsItKernel::KernelSystem(this->GetWorld());
	kernel->setNetwork(new FicsItKernel::Network::NetworkController());
	kernel->getNetwork()->component = NetworkConnector;

	recalculateKernelResources();
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
		} else {
			if (kernel->getProcessor()) kernel->setProcessor(nullptr);
			else recalculateKernelResources();
		}
	} else if (AFINComputerMemory* memory = Cast<AFINComputerMemory>(module)) {
		kernel->setCapacity(kernel->getCapacity() + (((added) ? 1 : -1) * memory->GetCapacity()));
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
		break;
	case FicsItKernel::KernelState::CRASHED:
		kernel->start(true);
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
