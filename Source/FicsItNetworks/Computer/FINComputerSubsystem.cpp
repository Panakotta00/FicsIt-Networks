#include "FINComputerSubsystem.h"

#include "Subsystem/SubsystemActorManager.h"
#include "FGCharacterPlayer.h"

AFINComputerSubsystem::AFINComputerSubsystem() {
	SetActorTickEnabled(true);
	PrimaryActorTick.SetTickFunctionEnable(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetReplicates(true);
	bAlwaysRelevant = true;
}

bool AFINComputerSubsystem::ShouldSave_Implementation() const {
	return true;
}

void AFINComputerSubsystem::PreSaveGame_Implementation(int32 saveVersion, int32 gameVersion) {
	Version = FINLatestVersion;
}

AFINComputerSubsystem* AFINComputerSubsystem::GetComputerSubsystem(UObject* WorldContext) {
#if WITH_EDITOR
	return nullptr;
#endif
	UWorld* WorldObject = GEngine->GetWorldFromContextObjectChecked(WorldContext);
	USubsystemActorManager* SubsystemActorManager = WorldObject->GetSubsystem<USubsystemActorManager>();
	check(SubsystemActorManager);
	return SubsystemActorManager->GetSubsystemActor<AFINComputerSubsystem>();
}
