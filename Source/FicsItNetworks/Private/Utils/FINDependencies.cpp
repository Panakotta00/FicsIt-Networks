#include "Utils/FINDependencies.h"

UE_DISABLE_OPTIMIZATION_SHIP
bool UFINDependency::AreDependenciesMet(UObject* worldContext) const {
	return AFIRSubsystem::GetReflectionSubsystem(worldContext)->IsAnyConnectorBlocked();
}
UE_ENABLE_OPTIMIZATION_SHIP