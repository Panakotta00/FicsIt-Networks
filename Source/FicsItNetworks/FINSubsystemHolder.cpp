#include "FINSubsystemHolder.h"

void UFINSubsystemHolder::InitSubsystems() {
	SpawnSubsystem(ComputerSubsystem, AFINComputerSubsystem::StaticClass(), "FINComputerSubsystem");
	SpawnSubsystem(HookSubsystem, AFINHookSubsystem::StaticClass(), "FINHookSubsystem");
	SpawnSubsystem(SignalSubsystem, AFINSignalSubsystem::StaticClass(), "FINSignalSubsystem");
}
