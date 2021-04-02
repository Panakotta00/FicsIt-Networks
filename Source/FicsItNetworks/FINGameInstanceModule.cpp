#include "FINGameInstanceModule.h"

#include "FINSubsystemHolder.h"

UFINGameInstanceModule::UFINGameInstanceModule() {
	ModSubsystems.Add(UFINSubsystemHolder::StaticClass());
}
