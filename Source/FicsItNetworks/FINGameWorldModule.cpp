#include "FINGameWorldModule.h"

#include "Computer/FINComputerSubsystem.h"
#include "Network/FINHookSubsystem.h"
#include "Network/Signals/FINSignalSubsystem.h"

UFINGameWorldModule::UFINGameWorldModule() {
	ModSubsystems.Add(AFINComputerSubsystem::StaticClass());
	ModSubsystems.Add(AFINSignalSubsystem::StaticClass());
	ModSubsystems.Add(AFINHookSubsystem::StaticClass());
}
