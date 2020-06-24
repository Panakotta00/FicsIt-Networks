#include "FINComputerSubsystem.h"

#include "FINSubsystemHolder.h"

bool AFINComputerSubsystem::ShouldSave_Implementation() const {
	return true;
}

AFINComputerSubsystem* AFINComputerSubsystem::GetComputerSubsystem(UObject* WorldContext) {
	return GetSubsystemHolder<UFINSubsystemHolder>(WorldContext)->ComputerSubsystem;
}
