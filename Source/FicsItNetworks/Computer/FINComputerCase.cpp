#include "FINComputerCase.h"

#include "FicsItKernel/Processor/Lua/LuaProcessor.h"

AFINComputerCase::AFINComputerCase() {
	NetworkConnector = CreateDefaultSubobject<UFINNetworkConnector>("NetworkConnector");
	NetworkConnector->AddMerged(this);
}

AFINComputerCase::~AFINComputerCase() {
	if (kernel) delete kernel;
}

void AFINComputerCase::BeginPlay() {
	kernel = new FicsItKernel::KernelSystem(this->GetWorld());
	kernel->setProcessor(new FicsItKernel::Lua::LuaProcessor());
	kernel->setNetwork(new FicsItKernel::Network::NetworkController());
	kernel->getNetwork()->component = NetworkConnector;
}

void AFINComputerCase::Factory_Tick(float dt) {
	kernel->tick(dt);
}

bool AFINComputerCase::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerCase::Toggle() {
	((FicsItKernel::Lua::LuaProcessor*)kernel->getProcessor())->setCode(TCHAR_TO_UTF8(*Code));
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
