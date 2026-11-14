#include "Utils/FINDependencies.h"

UE_DISABLE_OPTIMIZATION_SHIP
bool UFINDependency::AreDependenciesMet(UObject* worldContext) const {
	AFIRSubsystem* Subsystem = AFIRSubsystem::GetReflectionSubsystem(worldContext);
	return Subsystem && Subsystem->IsAnyConnectorBlocked();
}
UE_ENABLE_OPTIMIZATION_SHIP