#include "FINSubsystemHolder.h"

void UFINSubsystemHolder::InitSubsystems() {
	SpawnSubsystem(ComputerSubsystem, AFINComputerSubsystem::StaticClass(), "ComputerSubsystem");
}
