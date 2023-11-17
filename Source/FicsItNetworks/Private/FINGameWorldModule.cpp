#include "FINGameWorldModule.h"
#include "Computer/FINComputerSubsystem.h"
#include "Network/FINHookSubsystem.h"
#include "Network/Signals/FINSignalSubsystem.h"
#include "Network/Wireless/FINWirelessSubsystem.h"
#include "Utils/FINBlueprintParameterHooks.h"

UFINGameWorldModule::UFINGameWorldModule() {
	ModSubsystems.Add(AFINComputerSubsystem::StaticClass());
	ModSubsystems.Add(AFINSignalSubsystem::StaticClass());
	ModSubsystems.Add(AFINHookSubsystem::StaticClass());
	ModSubsystems.Add(AFINWirelessSubsystem::StaticClass());
	ModSubsystems.Add(AFINBlueprintParameterHooks::StaticClass());
}
