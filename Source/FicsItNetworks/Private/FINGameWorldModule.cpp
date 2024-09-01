#include "FINGameWorldModule.h"

#include "FicsItLogLibrary.h"
#include "FIRModModule.h"
#include "Computer/FINComputerSubsystem.h"
#include "Network/Signals/FINSignalSubsystem.h"
#include "Network/Wireless/FINWirelessSubsystem.h"
#include "Utils/FINBlueprintParameterHooks.h"
#include "Utils/FINMediaSubsystem.h"

UFINGameWorldModule::UFINGameWorldModule() {
	ModSubsystems.Add(AFINComputerSubsystem::StaticClass());
	ModSubsystems.Add(AFINSignalSubsystem::StaticClass());
	ModSubsystems.Add(AFINWirelessSubsystem::StaticClass());
	ModSubsystems.Add(AFINBlueprintParameterHooks::StaticClass());
	ModSubsystems.Add(AFINMediaSubsystem::StaticClass());
}

void UFINGameWorldModule::DispatchLifecycleEvent(ELifecyclePhase Phase) {
	Super::DispatchLifecycleEvent(Phase);

	switch (Phase) {
	case ELifecyclePhase::CONSTRUCTION:
		SpawnChildModule(TEXT("FicsItReflection"), UFIRGameWorldModule::StaticClass());
		break;
	default: break;
	}
}
